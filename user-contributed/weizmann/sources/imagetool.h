#define   VALUE        (caddr_t)'v'
#define   PIXEL        (caddr_t)'p'
#define   SUBREGION    (caddr_t)'s'
#define   HISTOGRAM    (caddr_t)'h'
#define   ZOOM         (caddr_t)'z'
#define   TWO          (caddr_t)'2'
#define   FOUR         (caddr_t)'4'
#define   EIGHT        (caddr_t)'8'
#define   OTHER        (caddr_t)'t'
#define   REFRESH      (caddr_t)'r'
#define   SAVE         (caddr_t)'a'
#define   QUIT         (caddr_t)'q'
#define   EXECUTE      (caddr_t)'e'
#define   EXIT         (caddr_t)'x'
#define   CLEAR        (caddr_t)'c'
#define   BUFFER       (caddr_t)'b'
#define   SFILE        (caddr_t)'f'
#define   KEY_BOARD    (caddr_t)'k'
#define   IMAGE_PIC    (caddr_t)'i'
#define   OUT          (caddr_t)'o'
#define   PHOTO        (caddr_t)'g'
#define   FULL         (caddr_t)'u'
#define   HELP         (caddr_t)'m'
#define   PREV         (caddr_t)'n'
#define   DES          (caddr_t)'w'

#define   FIRST_CORNER     1
#define   SECOND_CORNER    2
#define   CORRECTION       3
#define   EXITMENU_SIZE    4
#define   PROCMENU_SIZE    10
#define   ZOOMENU_SIZE     4
#define   FRESHMENU_SIZE   3
#define   SAVEMENU_SIZE    3
#define   DESMENU_SIZE     3
struct subregion {
       int onscreen;
       int dimension_ok;
       int origin_x;
       int origin_y;
       int lowcorner_x;
       int lowcorner_y;
 };
struct desplace{
    int loc_x;
    int loc_y;
};

#define   MAX_TABLE_XLEN  10
#define   MAX_TABLE_YLEN  10
#define   MAX_TABLESIZE   MAX_TABLE_XLEN * MAX_TABLE_YLEN
#define   MAX_COMMANDLEN  200
#define   MAX_FILENAMELEN 25
#define   CURSOR_XLEN     5
#define   CURSOR_YLEN     5
#define   CURSOR_SIZE     CURSOR_XLEN * CURSOR_YLEN
#define   OPEN            3
