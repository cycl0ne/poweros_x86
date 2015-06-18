//declaration

#define MODF_S 1
#define MODF_K 2
#define MODF_M 4
#define MODF_N 8
#define MODF_A 16

#define MAX_OPTS 64
#define MAX_ARGS 128
#define BUF_CHUNK 512

struct data
{
    struct MinList opt_list;
    struct opt_node* opt_mem;
    struct MinList arg_list;
    struct arg_node* arg_mem;
    UINT32* all_mem;
    UINT32* free_mem;
};

struct opt_node
{
  struct MinNode o_node;
  UINT8* o_start;
  UINT8* o_end;

  UINT32 o_modf;
  UINT32 o_temp_res; //while processing, keep the result here
  UINT32* o_res; //when processing is over, do *o_res = o_temp_res;
  UINT8 o_processed; //opt processed, don't look at it again
  UINT8 o_has_result; //user has input some value for it

  struct arg_node* o_arg_start;
};

struct arg_node
{
  struct MinNode a_node;
  UINT8* a_start;
  UINT8* a_end;
  UINT8 a_keyword;
};

struct RdArgs
{
  UINT8* inputBuf;
  UINT32* memBuf;
};

#if 0
struct RdArgs* rdargs(UINT8* opt, UINT32* res);
void freeargs(struct RdArgs* ret);

static BOOL create_opt_list(UINT8* opt, UINT32* res, struct data* d);
static BOOL create_arg_list(UINT8* arg, struct data* d);
static BOOL equal(UINT8* src1_start, UINT8* src1_end, UINT8* src2_start, UINT8* src2_end);
static void consume_arg(UINT32* arg, UINT8* curr_arg_start);
static UINT8* getnext_modifier(UINT8* curr_modf);
static void curr_opt(UINT8* start, UINT8** curr_opt_start, UINT8**curr_opt_end);
static void curr_arg(UINT8* start, UINT8** curr_arg_start, UINT8**curr_arg_end);
static void next_opt(UINT8* start, UINT8** next_opt_start, UINT8**next_opt_end);
static UINT8 count_string_args(struct opt_node* opt);
static void steal_one_string_arg(struct opt_node* prev_opt, struct opt_node* opt);
static BOOL mark_keywords(struct data* d);
static BOOL collect_args_with_keyword(struct data* d);
static BOOL collect_args_without_keyword();
static BOOL adjust_string_args(struct data* d);
static UINT8* read_input(UINT8* opt);
static UINT32* alloc_mem();
static BOOL convert_to_long(UINT8* p_digit, INT32* p_val);
static BOOL convert_numeric_string_args(struct data* d);
static BOOL check_mandatory_opts(struct data* d);
static BOOL save_result(struct data* d);
static BOOL check_set_opts(struct data* d);
static BOOL mark_keyword_for_opt(struct opt_node* opt, struct data* d);
static BOOL consume_one_arg(struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword);
static BOOL consume_multiple_arg(struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword, struct data* d);
static BOOL confirm_no_extra_args(struct data* d);
static BOOL unquote_string_args(struct data* d);
static BOOL unquote_arg(UINT32* p_arg);
void print_result(struct data* d);
void print_opt_list();
void print_arg_list();
#endif
