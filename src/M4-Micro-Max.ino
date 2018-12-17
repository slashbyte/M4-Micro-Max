/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/***************************************************************************/
/* version 4.8 (1953 characters) features:                                 */
/* - recursive negamax search                                              */
/* - all-capture MVV/LVA quiescence search                                 */
/* - (internal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - a hash table storing score and best move                              */
/* - futility pruning                                                      */
/* - king safety through magnetic, frozen king in middle-game              */
/* - R=2 null-move pruning                                                 */
/* - keep hash and repetition-draw detection                               */
/* - better defense against passers through gradual promotion              */
/* - extend check evasions in inner nodes                                  */
/* - reduction of all non-Pawn, non-capture moves except hash move (LMR)   */
/* - full FIDE rules (expt under-promotion) and move-legality checking     */
/***************************************************************************/
/*                               micro-Max,                                */
/*                            ported to arduino                            */
/*                             Slash/Byte                                  */
/***************************************************************************/
/* - reduced hash table size to 2K entries  9-10-2018                      */
/* - fixed userinput for arduino                                           */
/* - fixed hasing for arduino  9-9-2018                                    */
/* - hash translation table now in flash  9-10-2018                        */
/* - root: deepen upto 1e5                                                 */
/***************************************************************************/

#define W while

#define U (1 << 13) //hash table 8k entries
struct _
{
  int32_t K, V;
  int8_t X, Y, D;
} A[U]; /* hash table, 16M+8 entries*/

int32_t M = 136, S = 128, I = 8e3, Q, O, K, N, R, J, Z, k = 16, *p, c[9]; /* M=0x88 */

int8_t L,
    w[] = {0, 2, 2, 7, -1, 8, 12, 23},                                      /* relative piece values     */
    o[] = {-16, -15, -17, 0, 1, 16, 0, 1, 16, 15, 17, 0, 14, 18, 31, 33, 0, /* step-vector lists         */
           7, -1, 11, 6, 8, 3, 6,                                           /* 1st dir. in o[] per piece */
           6, 3, 5, 7, 4, 5, 3, 6},                                         /* initial piece setup       */
    b[129];                                                                 /* board: half of 16x8+dummy */
    //T[1035];/* hash translation table   */

char buffer[100] = {0}; //for post display


//size is 1035
/* hash translation table   */
const int8_t T[] =
    {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 32, 10, 46, 25, 17, 25, 59,
     19, 20, 30, 20, 35, 48, 43, 4, 10, 30, 37, 13, 14, 24, 4, 12, 30, 47,
     44, 9, 27, 10, 54, 63, 41, 48, 49, 55, 57, 47, 12, 47, 37, 14, 31, 47,
     13, 23, 6, 17, 24, 41, 16, 6, 63, 30, 33, 45, 6, 49, 29, 9, 17, 58,
     45, 36, 44, 36, 52, 35, 62, 48, 29, 46, 1, 56, 48, 36, 35, 51, 49, 56,
     5, 51, 0, 60, 42, 41, 26, 45, 15, 26, 21, 15, 2, 23, 41, 35, 43, 30,
     23, 0, 31, 61, 13, 51, 58, 30, 29, 21, 1, 11, 16, 62, 32, 26, 58, 18,
     63, 13, 32, 12, 52, 17, 0, 50, 34, 54, 32, 37, 61, 61, 30, 7, 4, 24,
     2, 2, 50, 2, 54, 13, 49, 0, 40, 47, 8, 58, 5, 23, 29, 58, 9, 19,
     2, 54, 15, 48, 23, 4, 38, 31, 12, 24, 20, 27, 6, 31, 2, 41, 33, 35,
     12, 0, 51, 13, 31, 37, 49, 40, 44, 1, 25, 59, 13, 20, 23, 37, 51, 52,
     54, 10, 52, 14, 20, 0, 2, 8, 48, 7, 52, 63, 14, 57, 48, 14, 55, 8,
     63, 2, 31, 50, 55, 51, 54, 51, 5, 44, 38, 50, 32, 19, 11, 42, 36, 57,
     34, 2, 51, 5, 1, 34, 36, 39, 19, 53, 55, 1, 31, 32, 50, 50, 40, 32,
     54, 55, 44, 32, 44, 2, 48, 45, 12, 10, 15, 32, 50, 49, 21, 33, 55, 50,
     41, 26, 62, 11, 28, 4, 56, 11, 62, 30, 5, 45, 54, 53, 55, 4, 34, 25,
     32, 7, 12, 30, 46, 1, 48, 3, 59, 17, 31, 30, 29, 27, 10, 52, 16, 35,
     40, 12, 13, 8, 29, 37, 40, 49, 2, 45, 11, 57, 53, 16, 33, 53, 32, 9,
     63, 12, 14, 2, 0, 0, 4, 51, 56, 51, 54, 16, 12, 46, 6, 45, 14, 2,
     51, 51, 43, 10, 26, 63, 6, 32, 49, 17, 15, 19, 37, 42, 60, 36, 58, 40,
     34, 44, 26, 18, 28, 25, 52, 36, 2, 54, 25, 1, 26, 39, 41, 31, 1, 12,
     27, 27, 60, 44, 20, 2, 46, 46, 38, 57, 35, 50, 54, 25, 37, 15, 7, 42,
     31, 22, 60, 5, 40, 14, 20, 27, 15, 21, 30, 25, 63, 20, 54, 44, 5, 59,
     15, 34, 36, 31, 0, 38, 50, 4, 58, 20, 48, 44, 5, 33, 24, 29, 56, 31,
     20, 21, 61, 10, 13, 15, 30, 46, 28, 13, 47, 22, 58, 21, 6, 12, 56, 9,
     35, 29, 44, 22, 43, 49, 9, 16, 33, 26, 35, 10, 3, 57, 62, 50, 45, 22,
     56, 5, 42, 3, 49, 39, 24, 56, 55, 43, 12, 50, 19, 26, 15, 2, 20, 11,
     35, 33, 28, 20, 55, 48, 28, 36, 43, 52, 1, 24, 14, 0, 51, 16, 52, 22,
     57, 32, 26, 42, 23, 24, 42, 35, 2, 7, 24, 53, 48, 48, 45, 50, 52, 23,
     54, 55, 32, 26, 48, 9, 26, 1, 27, 5, 37, 51, 33, 1, 16, 61, 44, 63,
     59, 35, 23, 6, 48, 18, 34, 14, 7, 0, 6, 8, 63, 6, 34, 0, 16, 9,
     4, 15, 38, 53, 36, 27, 56, 27, 8, 56, 54, 15, 26, 4, 4, 59, 62, 30,
     46, 6, 36, 32, 36, 26, 31, 38, 12, 22, 12, 21, 14, 53, 10, 51, 38, 2,
     7, 49, 22, 31, 47, 6, 12, 49, 62, 61, 20, 58, 61, 5, 45, 56, 40, 57,
     18, 13, 24, 36, 14, 7, 27, 56, 62, 11, 25, 62, 61, 3, 54, 50, 43, 1,
     18, 21, 20, 6, 9, 45, 17, 34, 34, 50, 1, 5, 9, 31, 30, 12, 4, 55,
     26, 33, 6, 40, 6, 38, 55, 20, 47, 28, 55, 1, 38, 43, 29, 36, 51, 62,
     59, 14, 35, 35, 45, 52, 31, 24, 51, 19, 24, 58, 42, 46, 35, 55, 23, 27,
     15, 6, 32, 49, 34, 58, 30, 61, 56, 51, 46, 16, 49, 16, 38, 20, 50, 19,
     18, 53, 30, 35, 38, 17, 19, 32, 63, 48, 3, 41, 41, 3, 3, 26, 62, 43,
     32, 21, 21, 62, 20, 23, 22, 13, 54, 62, 47, 45, 41, 52, 53, 57, 56, 35,
     50, 13, 33, 49, 45, 63, 52, 28, 42, 11, 13, 55, 48, 52, 17, 47, 20, 21,
     46, 2, 22, 43, 13, 22, 12, 2, 54, 50, 45, 29, 8, 31, 59, 54, 24, 13,
     17, 57, 17, 2, 25, 26, 29, 31, 5, 22, 3, 21, 41, 14, 61, 5, 36, 18,
     36, 45, 6, 55, 8, 34, 4, 26, 12, 40, 55, 53, 17, 59, 54, 19, 24, 23,
     8, 15, 33, 59, 0, 54, 36, 27, 48, 49, 57, 30, 36, 7, 8, 37, 55, 38,
     16, 4, 51, 0, 34, 24, 43, 46, 36, 45, 28, 29, 42, 31, 27, 2, 13, 33,
     19, 18, 5, 34, 27, 59, 30, 52, 49, 38, 43, 38, 12, 38, 1, 1, 40, 60,
     14, 20, 49, 61, 45, 0, 53, 3, 8, 42, 7, 8, 21, 15, 37, 3, 24, 20,
     43, 40, 39, 25, 19, 38, 32, 38, 4, 9, 16, 45, 1, 20, 49, 12, 52, 57,
     10, 57, 36, 7, 21, 18, 19, 46, 46};

/* recursive minimax search, k=moving side, n=depth*/
int32_t D(int32_t q, int32_t l, int32_t e, int32_t E, int32_t z, int32_t n) /* (q,l)=window, e=current eval. score, E=e.p. sqr.*/
{                                                                           /* e=score, z=prev.dest; J,Z=hashkeys; return score*/
  int32_t j, r, m, v, d, h, i, F, G, V, P, f = J, g = Z, C, s;
  int32_t tmp = 0, bJ = 0; //for hash fix
  int8_t t, p, u, x, y, X, Y, H, B;
  //pointer to const int8_t
  int8_t const *tmq;                     //for hash fix
  struct _ *a = A + (J + k * E & U - 1); /* lookup pos. in hash table*/

  q--;     /* adj. window: delay bonus */
  k ^= 24; /* change sides             */
  d = a->D;
  m = a->V;
  X = a->X;
  Y = a->Y; /* resume at stored depth   */

  if (a->K - Z | z |                       /* miss: other pos. or empty*/
      !(m <= q | X & 8 && m >= l | X & S)) /*   or window incompatible */
    d = Y = 0;                             /* start iter. from scratch */
  X &= ~M;                                 /* start at best-move hint  */

  W(d++ < n || d < 3 ||                         /* iterative deepening loop */
    z & K == I && (N < 1e5 & d < 98 ||          /* root: deepen upto time   */
                   (K = X, L = Y & ~M, d = 3))) /* time's up: go do best    */
  {
    x = B = X;                                     /* start scan at prev. best */
    h = Y & S;                                     /* request try noncastl. 1st*/
    P = d < 3 ? I : D(-l, 1 - l, -e, S, 0, d - 3); /* Search null move         */
    m = -P<l | R> 35 ? d > 2 ? -I : e : -P;        /* Prune or stand-pat       */
    N++;                                           /* node count (for timing)  */
    do
    {
      u = b[x];  /* scan board looking for   */
      if (u & k) /*  own piece (inefficient!)*/
      {
        r = p = u & 7;                      /* p = piece type (set r>0) */
        j = o[p + 16];                      /* first step vector f.piece*/
        W(r = p > 2 & r < 0 ? -r : -o[++j]) /* loop over directions o[] */
        {
        A: /* resume normal after best */
          y = x;
          F = G = S; /* (x,y)=move, (F,G)=castl.R */
          do
          {                            /* y traverses ray, or:     */
            H = y = h ? Y ^ h : y + r; /* sneak in prev. best move */
            if (y & M)
              break;                                           /* board edge hit           */
            m = E - S & b[E] && y - E < 2 & E - y < 2 ? I : m; /* bad castling             */
            if (p < 3 & y == E)
              H ^= 16; /* shift capt.sqr. H if e.p.*/
            t = b[H];
            if (t & k | p < 3 & !(y - x & 7) - !t)
              break;                       /* capt. own, bad pawn mode */
            i = 37 * w[t & 7] + (t & 192); /* value of capt. piece t   */
            m = i < 0 ? I : m;             /* K capture                */
            if (m >= l & d > 1)
              goto C; /* abort on fail high       */

            v = d - 1 ? e : i - p; /* MVV/LVA scoring          */
            if (d - !t > 1)        /* remaining depth          */
            {
              v = p < 6 ? b[x + 8] - b[y + 8] : 0; /* center positional pts.   */
              b[G] = b[H] = b[x] = 0;
              b[y] = u | 32; /* do move, set non-virgin  */
              if (!(G & M))
                b[F] = k + 6, v += 50;      /* castling: put R & score  */
              v -= p - 4 | R > 29 ? 0 : 20; /* penalize mid-game K move */
              if (p < 3)                    /* pawns:                   */
              {
                v -= 9 * ((x - 2 & M || b[x - 2] - u) +              /* structure, undefended    */
                          (x + 2 & M || b[x + 2] - u) - 1            /*        squares plus bias */
                          + (b[x ^ 16] == k + 36))                   /* kling to non-virgin King */
                     - (R >> 2);                                     /* end-game Pawn-push bonus */
                V = y + r + 1 & S ? 647 - p : 2 * (u & y + 16 & 32); /* promotion or 6/7th bonus */
                b[y] += V;
                i += V; /* change piece, add score  */
              }
              v += e + i;
              V = m > q ? m : q; /* new eval and alpha       */

              /* wont jive with arduino
              J += J(0);
              Z += J(8) + G - S; // update hash key
              /* fix */

              tmq = (T + (y + 0) + ((b[y]) & 8) + S * ((b[y]) & 7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ = tmp;
              tmq = (T + (x + 0) + ((u)&8) + S * ((u)&7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ -= tmp;
              tmq = (T + (H + 0) + ((t)&8) + S * ((t)&7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ -= tmp;
              J += bJ; // update hash key fix

              tmq = (T + (y + 8) + ((b[y]) & 8) + S * ((b[y]) & 7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ = tmp;
              tmq = (T + (x + 8) + ((u)&8) + S * ((u)&7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ -= tmp;
              tmq = (T + (H + 8) + ((t)&8) + S * ((t)&7));
              memcpy(&tmp, tmq, sizeof(int32_t));
              bJ -= tmp;
              Z = Z + bJ + G - S; // update hash key fix

              C = d - 1 - (d > 5 & p > 2 & !t & !h);
              C = R > 29 | d < 3 | P - I ? C : d; /* extend 1 ply if in check */
              do
                s = C > 2 | v > V ? -D(-l, -V, -v, /* recursive eval. of reply */
                                       F, 0, C)
                                  : v; /* or fail low if futile    */
              W(s > q & ++C < d);
              v = s;
              if (z && K - I && v + I && x == K & y == L) /* move pending & in root:  */
              {
                Q = -e - i;
                O = F; /*   exit if legal & found  */
                a->D = 99;
                a->V = 0; /* lock game in hash as draw*/
                R += i >> 7;
                return l; /* captured non-P material  */
              }
              J = f;
              Z = g; /* restore hash key         */
              b[G] = k + 6;
              b[F] = b[y] = 0;
              b[x] = u;
              b[H] = t; /* undo move,G can be dummy */
            }
            if (v > m)                     /* new best, update max,best*/
              m = v, X = x, Y = y | S & F; /* mark double move with S  */
            if (h)
            {
              h = 0;
              goto A;
            }                                              /* redo after doing old best*/
            if (x + r - y | u & 32 |                       /* not 1st step,moved before*/
                p > 2 & (p - 4 | j - 7 ||                  /* no P & no lateral K move,*/
                         b[G = x + 3 ^ r >> 1 & 7] - k - 6 /* no virgin R in corner G, */
                         || b[G ^ 1] | b[G ^ 2])           /* no 2 empty sq. next to R */
            )
              t += p < 5; /* fake capt. for nonsliding*/
            else
              F = y; /* enable e.p.              */
          }
          W(!t); /* if not capt. continue ray*/
        }
      }
    }
    W((x = x + 9 & ~M) - B); /* next sqr. of board, wrap */
  C:
    if (m > I - M | m < M - I)
      d = 98;                                             /* mate holds to any depth  */
    m = m + I | P == I ? m : 0;                           /* best loses K: (stale)mate*/
    if (a->D < 99)                                        /* protect game history     */
      a->K = Z, a->V = m, a->D = d,                       /* always store in hash tab */
          a->X = X | 8 * (m > q) | S * (m < l), a->Y = Y; /* move, type (bound/exact),*/
    if (z)
    {
      sprintf(buffer, "%2d ply, %9d searched, score=%6d by %c%c%c%c", d - 1, N - S, m,
              'a' + (X & 7), '8' - (X >> 4), 'a' + (Y & 7), '8' - (Y >> 4 & 7));
      Serial.println(buffer); //for Kibitz!
    }

  }                  /*    encoded in X S,8 bits */
  k ^= 24;           /* change sides back        */
  return m += m < e; /* delayed-loss bonus       */
}

/* User interface routines */
void myputchar(char c)
{
  Serial.write(c);
}

void myputs(const char *s)
{
  while (*s)
    myputchar(*s++);
  myputchar('\n');
}

char mygetchar(void)
{
  while (!Serial.available())
    ;
  if (Serial.available() > 0)
    return Serial.read();
  else
    return 10;
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("Micro-Max by H.G. Muller");
  Serial.println("A port for Adafruit's ItsyBitsy M4");
  Serial.println("Slash/Byte\n");
    
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);                      // wait for a second
}

void loop()
{
  int32_t startTime;

  K = 8;
  c[0] = 10;
  W(K--)
  {
    b[K] = (b[K + 112] = o[K + 24] + 8) + 8;
    b[K + 16] = 18;
    b[K + 96] = 9; /* initial board setup*/
    L = 8;
    W(L--)
    b[16 * L + K + 8] = (K - 4) * (K - 4) + (L - 3.5) * (L - 3.5); /* center-pts table   */
  }                                                                /*(in unused half b[])*/

  W(1) /* play loop */
  {
    N = -1;
    W(++N < 121)
    myputchar(N & 8 && (N += 7) ? 10 : ".?+nkbrq?*?NKBRQ"[b[N] & 15]); /* print board */

    if (k != 16)
    {
      p = c;
      // read input line
      W((*p++ = mygetchar()) > 10);
    }
    else
    {
      c[0] = 10;
    }

    K = I; /* invalid move */
    if (*c - 10)
      K = *c - 16 * c[1] + 799, L = c[2] - 16 * c[3] + 799; /* parse entered move */
    startTime = millis();
    D(-I, I, Q, O, 1, 3); /* think or check & do*/
    sprintf(buffer, "Time: Took %d sec", (millis() - startTime) / 1000);
    Serial.println(buffer);
    digitalWrite(LED_BUILTIN, LOW);  //blue on
    sprintf(buffer, "move: %c%c%c%c", 'a' + (K & 7), '8' - (K >> 4), 'a' + (L & 7), '8' - (L >> 4));
    Serial.println(buffer);
  }
}


