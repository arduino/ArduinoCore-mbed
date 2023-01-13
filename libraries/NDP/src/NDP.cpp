#include "Arduino.h"
#include "NDP.h"
#include "mbed_events.h"
#include "mbed_shared_queues.h"

#define NDP_SPICTL  0x4000903c
// register for data transmission
#define NDP_SPITX_Start 0x40009040
#define NDP_SPITX_0 0x40009040
#define NDP_SPITX_1 0x40009044
#define NDP_SPITX_2 0x40009048
#define NDP_SPITX_3 0x4000904c
// register for data reception
#define NDP_SPIRX_0 0x40009050
#define NDP_SPIRX_1 0x40009054
#define NDP_SPIRX_2 0x40009058
#define NDP_SPIRX_3 0x4000905c
#define NDP_SPIRX_Start 0x40009050
#define burstWriteBytes 14

static SPIFBlockDevice spif(SPI_PSELMOSI0, SPI_PSELMISO0, SPI_PSELSCK0, CS_FLASH, 16000000);
static mbed::LittleFileSystem fs("fs");
static events::EventQueue queue(10 * EVENTS_EVENT_SIZE);
static rtos::Thread event_t(osPriorityAboveNormal, 768, nullptr, "events");

uint32_t NDPClass::spi_speed_general = 12000000;
uint32_t NDPClass::spi_speed_initial = 1000000;
int NDPClass::pdm_clk_init = 0;

static struct syntiant_ndp120_tiny_device_s _ndp;
static struct syntiant_ndp120_tiny_device_s *ndp = &_ndp;

struct ndp120_fll_preset_s {
  const char *name;
  int operating_voltage;
  uint32_t input_freq;
  uint32_t output_freq;
  uint32_t pdm_freq;
};

enum {
  PLL_PRESET_OP_VOLTAGE_0p9 = 0,
  PLL_PRESET_OP_VOLTAGE_1p0,
  PLL_PRESET_OP_VOLTAGE_1p1
};

/* Define the table of FLL settings */
static struct ndp120_fll_preset_s ndp120_fll_presets[] = {
    {"mode_fll_0p9v_15p360MHz_32p768kHz", PLL_PRESET_OP_VOLTAGE_0p9, 32768, 15360000, 768000},
    {"mode_fll_0p9v_16p896MHz_32p768kHz", PLL_PRESET_OP_VOLTAGE_0p9, 32768, 16896000, 768000},
    { NULL, 0, 0, 0, 0}
};

static char fwver[NDP120_MCU_FW_VER_MAX_LEN] = "";
static char dspfwver[NDP120_MCU_DSP_FW_VER_MAX_LEN] = "";
static char pkgver[NDP120_MCU_PKG_VER_MAX_LEN] = "";
static uint8_t pbiver[NDP120_MCU_PBI_VER_MAX_LEN] = "";

#define SYNTIANT_NDP120_ERROR_NAMES                                            \
    {                                                                          \
        "none", "fail", "arg", "uninit", "package", "unsup", "nomem", "busy",  \
        "timeout", "more", "config", "crc", "inv_net", "reread", "pbi_tag",    \
        "pbi_ver" , "invalid _length", "dsp_hdr_crc"                           \
    }

const char *syntiant_ndp_error_names[] = SYNTIANT_NDP120_ERROR_NAMES;

/* error handling function */
#define check_status(message, s, do_exit) do { \
  if (s) { \
    printf("%s failed: %s\n", message, syntiant_ndp_error_names[s]); \
    if (on_error_cb) on_error_cb(); \
    if (do_exit) { return 0; } \
  } \
} while (0); \

static rtos::Mutex mtx;

int NDPClass::spiTransfer(void *d, int mcu, uint32_t address, void *_out, void *_in, unsigned int count) {

  mtx.lock();
  uint8_t *out = (uint8_t *)_out;
  uint8_t *in = (uint8_t *)_in;
  int s = SYNTIANT_NDP120_ERROR_NONE;
  uint8_t dummy[4] = {0};
  unsigned int i;
  uint8_t addr_cmd[5];

  if (in && out) {
    return SYNTIANT_NDP120_ERROR_ARG;
  }

  if (mcu) {
    if ((count & 0x3) != 0) {
      return SYNTIANT_NDP120_ERROR_ARG;
    }
    if (out) {
      digitalWrite(NDP_CS, LOW);
      addr_cmd[0] = NDP120_SPI_MADDR(0);
      memcpy(&addr_cmd[1], &address, sizeof(uint32_t));
      SPI1.transfer(addr_cmd, 5);
      SPI1.transfer(out, count); // NB: overwrites out
      digitalWrite(NDP_CS, HIGH);
    } else {
      digitalWrite(NDP_CS, LOW);
      addr_cmd[0] = NDP120_SPI_MADDR(0);
      memcpy(&addr_cmd[1], &address, sizeof(uint32_t));
      SPI1.transfer(addr_cmd, 5);
      digitalWrite(NDP_CS, HIGH);
      for (int i = 0; i < 4; i++); // short delay
      digitalWrite(NDP_CS, LOW);
      // Reading 4 dummy bytes allows for running SPI faster
      // as this gives HW state maching time to fetch result
      SPI1.transfer(0x80 | NDP120_SPI_MADDR(0));
      SPI1.transfer(dummy, 4);
      SPI1.transfer(&in[i], count);
      //Serial.println(*((uint32_t*)in), HEX);
      digitalWrite(NDP_CS, HIGH);
    }
  } else {
    if (out) {

      //nrf_gpio_pin_clear(SPI_CS);
      digitalWrite(NDP_CS, LOW);
      SPI1.transfer(address);
      SPI1.transfer(out, count);  // NB: overwrites out
      digitalWrite(NDP_CS, HIGH);
    } else {
      digitalWrite(NDP_CS, LOW);
      SPI1.transfer(0x80 | address);
      SPI1.transfer(in, count);
      digitalWrite(NDP_CS, HIGH);
    }
  }
  mtx.unlock();
  return s;
}

static long getFileSize(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  rewind(fp);

  return size;
}

static void addFilesFromFolder(char* path, char** list) {
  DIR *d = opendir(path);
  if (!d) {
    return;
  }

  int idx = 0;
  while (true) {
    struct dirent *e = readdir(d);
    if (!e) {
      break;
    }
    char* name = (char*)malloc(strlen(e->d_name));
    memcpy(name, e->d_name, strlen(e->d_name) + 1);
    list[idx++] = name;
  }
}

int NDPClass::sync(void *d)
{
  return SYNTIANT_NDP120_ERROR_NONE;
}

int NDPClass::unsync(void *d)
{
  return SYNTIANT_NDP120_ERROR_NONE;
}

/* iif mailbox-exchange wait. this implementation just polls for mailbox
   completion */
int NDPClass::mbwait(void *d)
{
  int s = SYNTIANT_NDP120_ERROR_NONE;
  int mbwait_count  = 0;
  uint32_t notifications = 0;
  int delay_time = 10;
  unsigned int time_start = millis();

  do {
    s = syntiant_ndp120_tiny_poll(ndp, &notifications, 1);
    if (s) return s;
    if (NDP.mbwait_timeout< (millis() - time_start)) {
      s = SYNTIANT_NDP120_ERROR_TIMEOUT;
      break;
    }
  } while (((notifications & SYNTIANT_NDP120_NOTIFICATION_MAILBOX_IN) == 0)
           && ((notifications & SYNTIANT_NDP120_NOTIFICATION_MAILBOX_OUT) == 0));
  return s;

}

int NDPClass::get_type(void *d, unsigned int *type)
{
  int s = SYNTIANT_NDP120_ERROR_NONE;
  uint8_t type_byte = 0x34;
  uint8_t data;
  s = spiTransfer(d, 0, NDP120_SPI_ID0, NULL, &data, 1);
  if (!s) {
    if (data) {
      type_byte = data;
    }
  }

  if (!s) {
    *type = type_byte;
  }
  return s;
}

static mbed::InterruptIn irq(NDPClass::NDP_INT, PullDown);

int NDPClass::getAudioChunkSize(void) {
  int s;
  uint32_t audio_chunk_size;
  s = syntiant_ndp120_tiny_get_audio_chunk_size(ndp, &audio_chunk_size);
  if (s) {
    return 0;
  }

  audio_sample_chunk_size = (unsigned int)
    (audio_chunk_size * SYNTIANT_NDP120_TINY_AUDIO_SAMPLE_RATE * SYNTIANT_NDP120_TINY_AUDIO_SAMPLES_PER_WORD / 1000000)
    + sizeof(struct syntiant_ndp120_tiny_dsp_audio_sample_annotation_t);
  return audio_sample_chunk_size;
}


int NDPClass::sendData(uint8_t *data, unsigned int len) 
{
  int s = syntiant_ndp120_tiny_send_data(ndp, data,
                                         len,
                                         SYNTIANT_NDP120_SEND_DATA_TYPE_STREAMING, 0);
  return s;
}



int NDPClass::extractData(uint8_t *data, unsigned int *len) {
  int s;
  unsigned int l = audio_sample_chunk_size;
  s = syntiant_ndp120_tiny_send_audio_extract(ndp, SYNTIANT_NDP120_EXTRACT_FROM_UNREAD);
  if (s) {
    *len = 0;
    return s;
  }
  // extract audio + annotations
  s = syntiant_ndp120_tiny_extract_data(ndp, data, &l);
  // in this simple case, don't return annotations to user
  l -= sizeof(syntiant_ndp120_tiny_dsp_audio_sample_annotation_t);
  *len = l;
  return s;
}

int NDPClass::poll() {
  uint32_t notifications;
  uint32_t summary;
  int match_class = -1;

  int  s = syntiant_ndp120_tiny_poll(ndp, &notifications, 1);
  check_status("syntiant_ndp_poll", s, 1);
  if (!(notifications & SYNTIANT_NDP120_NOTIFICATION_MATCH)) {
      delay(100);
      return 0;
  }

  s = syntiant_ndp120_tiny_get_match_summary(ndp, &summary);
  check_status("get_match_summary", s, 1);

  if ((summary & NDP120_SPI_MATCH_MATCH_MASK)) {
    match_class = NDP120_SPI_MATCH_WINNER_EXTRACT(summary);
    if (on_match_cb_s) {
      on_match_cb_s(labels[match_class]);
    }
    if (on_match_cb_i) {
      on_match_cb_i(match_class);
    }
  }
  return match_class+1;
}

void NDPClass::interrupt_handler() {
  if (!_initialized || !poll()) {
    if (on_event_cb) {
      on_event_cb();
    }
  }
}

int NDPClass::load(const char* fw, int bl) {
  fs.mount(&spif);
  static int loaded = 0;
  if (fw == nullptr) {
    return 0;
  }

  unsigned int file_length;
  unsigned int chunk_size = 2048;
  if (bl) {
    chunk_size = 1024;
  }
  char *file_data = (char *) malloc(sizeof(char) * chunk_size);
  if (!file_data) {
      printf("can't allocate memory for 1KB\n");
      return 0;
  }
  char tmp[100];
  // Reset package loading
  int s = syntiant_ndp120_tiny_load(ndp, NULL, 0);
  if (s != SYNTIANT_NDP120_ERROR_MORE) {
    strcpy(tmp, "Error resetting package load state: ");
    strcat(tmp, fw);
    check_status(tmp, s, 1);
  }
  // Load package
  String path = "/fs/" + String(fw);
  FILE* package = fopen(path.c_str(), "rb");
  if (package == NULL) {
    s = 1;
    strcpy(tmp, "Error opening: ");
    strcat(tmp, fw);
    check_status(tmp, s, 1);
  }
  memset(file_data, 0, chunk_size);
  file_length = getFileSize(package);
  while (chunk_size) {
    fread(file_data, 1, chunk_size, package);
    s = syntiant_ndp120_tiny_load(ndp, file_data, chunk_size);
    if (s != SYNTIANT_NDP120_ERROR_NONE && s != SYNTIANT_NDP120_ERROR_MORE) {
      strcpy(tmp, "Error loading ");
      strcat(tmp, fw);
      check_status(tmp, s, 1);
      break;
    }
    file_length -= chunk_size;
    if (file_length < chunk_size) {
      chunk_size = file_length;
    }
  }

  if (s) {
    strcpy(tmp, "Error: Loading ");
    strcat(tmp, fw);
    check_status(tmp, s, 1);
  }

  fclose(package);
  loaded++;
  if (file_data) {
    free(file_data);
  }
  if (file_length) {
    printf("Couldn't load package\n");
    return 0;
  }
  fs.unmount();
  spif.deinit();

  // after loading FW & DSP, we can configure the clk and increase SPI speed
  if (loaded == 2) {
    /* this delay can go away in an upcoming tiny ilib release
     * and instead be a loop waiting for DSP to start */
    delay(750);
    /* poll to process the DSP running indication */
    poll();
    configureClock();
    SPI1.endTransaction();
    SPI1.beginTransaction(SPISettings(spi_speed_general, MSBFIRST, SPI_MODE0));
  }

  if (loaded == 3) {
    _initialized = true;
  }
  return 1;
}

int NDPClass::enable_interrupts(bool enable) {
  int on;
  int s = SYNTIANT_NDP120_ERROR_NONE;
  if (enable) {
    on = SYNTIANT_NDP120_INTERRUPT_DEFAULT;
    if (!_int_pin_enabled) {
      event_t.start(callback(&queue, &events::EventQueue::dispatch_forever));
      irq.rise(queue.event(mbed::callback(this, &NDPClass::interrupt_handler)));
      _int_pin_enabled = true;
    }
    s = syntiant_ndp120_tiny_interrupts(ndp, &on);
    check_status("Error syntiant_ndp_interrupts", s, 1);
  } else {
    /* disable interrupt */
    on = 0;
    s = syntiant_ndp120_tiny_interrupts(ndp, &on);
  }
  return s;
}

int NDPClass::begin(const char* fw1) {
  memset(ndp, 0, sizeof(*ndp));
  // Prepare for SPI
  pinMode(PORSTB, OUTPUT);
  digitalWrite(PORSTB, LOW);
  delay(100);
  digitalWrite(PORSTB, HIGH);
  delay(100);

  SPI1.begin();
  SPI1.beginTransaction(SPISettings(spi_speed_initial, MSBFIRST, SPI_MODE0));

  // See which board we are by trying to read NDP120 Registion Register
  pinMode(NDP_CS, OUTPUT);
  digitalWrite(NDP_CS, HIGH);

  int s;

  /* setup the integration interface functions */
  iif.d = ndp;
  iif.mbwait = &NDPClass::mbwait;
  iif.sync = &NDPClass::sync;
  iif.unsync = &NDPClass::unsync;
  iif.transfer = &NDPClass::spiTransfer;

  /* initialize the ndp based on the interface functions */
  s = syntiant_ndp120_tiny_init(ndp, &iif, SYNTIANT_NDP_INIT_MODE_RESET);
  check_status("Error syntiant_ndp120_tiny_init", s, 1);

  load(fw1, 1);

  s = syntiant_cspi_init(ndp);
  check_status("Error syntiant_cspi_init", s, 1);

return 1;
}

int NDPClass::getInfo() {
  // get the match strings from the loaded model (if any)
  struct syntiant_ndp120_tiny_info info;
  info.fw_version = fwver;
  info.dsp_fw_version = dspfwver;
  info.pkg_version = pkgver;
  info.pbi = pbiver;
  info.labels = label_data;
  int s = syntiant_ndp120_tiny_get_info(ndp, &info);

  if (s) {
    printf("%d from syntiant_ndp120_tiny_get_info\n", s);
    return 0;
  }
  if (LABELS_STRING_LEN < info.labels_len) {
    printf("labels strings too long\n");
    return 0;
  }

  int num_labels = 0;
  /* get pointers to the labels */
  int j = 0;
  while ((info.labels_len - j > 3) && (num_labels < SYNTIANT_NDP120_MAX_CLASSES)) {
      labels[num_labels] = &label_data[j];
      num_labels++;
      for (; label_data[j]; j++)
          ;
      j++;
  }
  uint32_t *pbi_version;
  pbi_version = (uint32_t *)&pbiver[0];
  printf("dsp firmware version: %s\n", dspfwver);
  printf("package version: %s\n", pkgver);
  printf("pbi version: ");
  printf("%lu.",*pbi_version++);
  printf("%lu.",*pbi_version++);
  printf("%lu-",*pbi_version++);
  printf("%lu\n",*pbi_version);
  printf("num of labels: %d\n", num_labels);
  printf("labels: ");
  for (int i = 0; i < num_labels; i++) {
      printf("%s", labels[i]);
      if (i < num_labels - 1) {
          printf(", ");
      }
  }
  printf("\ntotal deployed neural networks: %d\n", info.total_nn);
  return 1;
}

int NDPClass::configureClock() {
  struct syntiant_ndp120_tiny_clk_config_data cfg;
  /* FLL mode: 0p9v_15p360MHz_32p768kHz, index 0 in the FLL table  */
  struct ndp120_fll_preset_s *fll_preset = &ndp120_fll_presets[0];
  cfg.src = SYNTIANT_NDP120_MAIN_CLK_SRC_FLL;
  cfg.core_freq = fll_preset->output_freq;
  cfg.voltage = fll_preset->operating_voltage;
  cfg.ref_type = 0; /* clk_pad */
  cfg.ref_freq = fll_preset->input_freq;
  int s = syntiant_ndp120_tiny_clock_cfg(ndp, &cfg);
  check_status("Error in clock config", s, 1);
  return (s == 0);
}

int NDPClass::configureInferenceThreshold(int threshold_bytes)
{
  int s = syntiant_ndp120_tiny_spi_direct_config(ndp, threshold_bytes);
  return s;
}

int NDPClass::checkMB() {
  uint32_t msg = 0;
  int s = syntiant_ndp120_tiny_mb_cmd(ndp, SYNTIANT_NDP120_MB_MCU_NOP, &msg);
  if (s) {
    printf("nop MB error: %s \n", SYNTIANT_NDP_ERROR_NAME(s));
  } else {
    printf("Host 2 MB successful\n");
  }

  s = syntiant_ndp120_tiny_mb_cmd(ndp, SYNTIANT_NDP120_MB_DSP_ADX_LOWER, &msg);
  if (s) {
    printf("Host 2 DSP MB error: %s \n", SYNTIANT_NDP_ERROR_NAME(s));
  } else {
    printf("Host 2 DSP successful\n");
  }
  return (s == 0);
}

int NDPClass::getDebugInfo() {
  int s = 0;
  struct syntiant_ndp120_tiny_debug cnt;
  memset(&cnt, 0, sizeof(cnt));
  s = syntiant_ndp120_tiny_get_debug(ndp, &cnt);
  if (s) {
    printf("Error %d from syntiant_ndp120_tiny_get_debug\n", s);
  } else {
      printf("Debug counters\n");
      printf("DSP counters:\n");
      printf("frame_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.frame_cnt);
      printf("dnn_int_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.dnn_int_cnt);
      printf("dnn_err_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.dnn_err_cnt);
      printf("h2d_mb_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.h2d_mb_cnt);
      printf("d2m_mb_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.d2m_mb_cnt);
      printf("m2d_mb_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.m2d_mb_cnt);
      printf("watermark_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.watermark_cnt);
      printf("fifo_overflow_cnt: 0x%lx\n", cnt.dsp_dbg_cnt.fifo_overflow_cnt);
      printf("MCU counters:\n");
      printf("signature: 0x%lx\n", cnt.mcu_dbg_cnt.signature);
      printf("frame_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.frame_cnt);
      printf("dsp2mcu_intr_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.dsp2mcu_intr_cnt);
      printf("dsp2mcu_nn_done_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.dsp2mcu_nn_done_cnt);
      printf("mcu2host_match_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.mcu2host_match_cnt);
      printf("mcu2host_mpf_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.mcu2host_mpf_cnt);
      printf("matches: 0x%lx\n", cnt.mcu_dbg_cnt.matches);
      printf("dsp2mcu_queue_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.dsp2mcu_queue_cnt);
      printf("mbin_int_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.mbin_int_cnt);
      printf("mbout_int_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.mbout_int_cnt);
      printf("nn_orch_flwchg_cnt: 0x%lx\n", cnt.mcu_dbg_cnt.nn_orch_flwchg_cnt);
      printf("unknown_activation_cnt: 0x%lx\n",  cnt.mcu_dbg_cnt.unknown_activation_cnt);
      printf("unknown_int_count: 0x%lx\n", cnt.mcu_dbg_cnt.unknown_int_count);
  }
  return !s;
}

int NDPClass::turnOnMicrophone() {
  int mode;
  int s = SYNTIANT_NDP120_ERROR_NONE;

  /* start the PDM clock clean for the first time and resume clock for
   * subsequent watches
   */
  if (!pdm_clk_init) {
    mode = SYNTIANT_NDP120_PDM_CLK_START_CLEAN;
    pdm_clk_init++;
  } else {
    mode = SYNTIANT_NDP120_PDM_CLK_START_RESUME;
  }
  s = syntiant_ndp120_tiny_pdm_clock_exe_mode(ndp, mode);
  return s;
}

int NDPClass::turnOffMicrophone() {
  return syntiant_ndp120_tiny_pdm_clock_exe_mode(ndp, SYNTIANT_NDP120_PDM_CLK_START_PAUSE);
}

int NDPClass::sensorBMI270Read(int reg, int len, uint8_t data_return_array[])
{
  int s;
  uint8_t cmd = 0x80 | reg;
  uint8_t dummy;

  s = syntiant_cspi_write(ndp, BMI270_SSB, 1, &cmd, 0);
  if (s) goto error;

  s = syntiant_cspi_read(ndp, BMI270_SSB, 1, &dummy, 0);
  if (s) goto error;

  s = syntiant_cspi_read(ndp, BMI270_SSB, len, data_return_array, 1);
  if (s) goto error;

error:
  return s;
}

int NDPClass::sensorBMI270Write(int reg, int len, uint8_t data_array[])
{
  int s;
  uint8_t cmd = reg;
  s = syntiant_cspi_write(ndp, BMI270_SSB, 1, &cmd, 0);
  if (s) goto error;

  s = syntiant_cspi_write(ndp, BMI270_SSB, len, data_array, 1);
  if (s) goto error;

error:
  return s;
}

int NDPClass::sensorBMI270Write(int reg, int data)
{
  int s;
  uint8_t cmd[2] = {reg, data & 0xff};
  s = syntiant_cspi_write(ndp, BMI270_SSB, 2, cmd, 1);
  return s;
}


int NDPClass::sensorBMM150Read(int reg, int len, uint8_t data_return_array[])
{
  int s;
  uint8_t cmd = 0x80 | reg;

  s = syntiant_cspi_write(ndp, BM150_SSB, 1, &cmd, 0);
  if (s) goto error;

  s = syntiant_cspi_read(ndp, BM150_SSB, len, data_return_array, 1);
  if (s) goto error;

error:
  return s;
}

int NDPClass::sensorBMM150Write(int reg, int len, uint8_t data_array[])
{
  int s;
  uint8_t cmd = reg;
  s = syntiant_cspi_write(ndp, BM150_SSB, 1, &cmd, 0);
  if (s) goto error;

  s = syntiant_cspi_write(ndp, BM150_SSB, len, data_array, 1);
  if (s) goto error;

error:
  return s;
}

int NDPClass::sensorBMM150Write(int reg, int data)
{
  int s;
  uint8_t cmd[2] = {reg, data & 0xff};
  s = syntiant_cspi_write(ndp, BM150_SSB, 2, cmd, 1);
  return s;
}

NDPClass NDP;
