
/*
 *                  Bit No:  7   6   5   4   3   2   1   0
 *                         +---+---+---+---+---+---+---+---+
 * Interrupt Vec    HIVR   |          V7 - V0              |
 *                         +---+---+-----------------------+
 * Cmd and Status   HCSR   | R | I |        A21 - A16      |
 *                         +---+---+-----------------------+
 * Addr High        HAHR   |             A15 - A8          |
 *                         +---------------------------+---+
 * Addr Low         HAHL   |          A7 - A1          | Q |
 *                         +---+---+---+---+---+---+---+---+
 *
 *                          Q = Abort
 *                          R = Ready
 *                          I = Interrupt Enable
 *
 * Command buffer address is set in Q,A1-A21.
 *    Q should always be 0, hence it must be word aligned.
 * Execution of command buffer is performed when R is set to 1.
 *
 * Each register is in its own word but only the least significant 8
 * bits are used.
 */

struct sbddevice {
  char hinv; /* Interrupt Vector        */
  char u0;   /* Unused                  */
  char hcsr; /* Cmd and Status Register */
  char u1;   /* Unused                  */
  char hahr; /* Host Address High       */
  char u2;   /* Unused                  */
  char hahl; /* Host Address Low        */
  char u3;   /* Unused                  */
};

/* SBD Register Bitmasks */
#define SBD_INTEN_HCSR 0200 /* _ I _ _ _ _ _ _ */
#define SBD_READY_HCSR 0400 /* R _ _ _ _ _ _ _ */
#define SBD_HADDR_HCSR 0077 /* _ _ A A A A A A */
#define SBD_HADDR_HAHR 0377 /* A A A A A A A A */
#define SBD_HADDR_HAHL 0376 /* A A A A A A A _ */
#define SBD_ABORT_HAHL 0001 /* _ _ _ _ _ _ _ Q */

/* SBD Function Codes */
#define SBD_FCRUB      0006 /* Erase rectangle                     */
#define SBD_FCPIX      0014 /* Display pixel graphics area         */
#define SBD_FCVEC      0027 /* Draw vector                         */
#define SBD_FCCIR      0032 /* Draw circle                         */
#define SBD_FCALP      0035 /* Display text on pixel area          */
#define SBD_FCWIB      0046 /* Write image block                   */
#define SBD_FCRIB      0047 /* Read image block                    */

