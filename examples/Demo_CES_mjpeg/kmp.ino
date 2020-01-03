void preKmp(char *x, int m, int kmpNext[])
{
   int i, j;

   i = 0;
   j = kmpNext[0] = -1;
   while (i < m)
   {
      while (j > -1 && x[i] != x[j])
         j = kmpNext[j];
      i++;
      j++;
      if (x[i] == x[j])
         kmpNext[i] = kmpNext[j];
      else
         kmpNext[i] = j;
   }
}

int KMP(char *x, int m, char *y, int n, int start)
{
   int i(0), j(start), kmpNext[m];

   /* Preprocessing */
   preKmp(x, m, kmpNext);

   /* Searching */
   while (j < n)
   {
      while (i > -1 && x[i] != y[j])
         i = kmpNext[i];
      i++;
      j++;
      if (i >= m)
         return (j - i);
   }
   return (-1);
}
