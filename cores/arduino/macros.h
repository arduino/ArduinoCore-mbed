#define analogPinToPinName(P)			(P < A0 ? g_APinDescription[P+A0].name : g_APinDescription[P].name)
#define digitalPinToPinName(P)			(g_APinDescription[P].name)
#define digitalPinToInterrupt(P) 		(P)
