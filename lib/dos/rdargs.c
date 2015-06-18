//when result of single-valued, optional, numeric opt
//shall be stored in result location directly
//rather than doing a malloc,
//it is hard to distinguish between two situations
//1. arg for that opt is supplied by user and value is 0
//2. arg for that opt is not supplied by user

//for this reason, when caller passes such an opt to readargs,
//and if default value of that opt is not 0,
//better caller set the default value in result array
//prior to calling readargs.


//After rdargs populates the result array, it looks like as follows:
//
//
//       opt/S    opt          opt/M                        opt/N        opt/M/N
//     +-------+------+------+-------+------+------+------+-------+------+------+--------+-------+
//     |   1   | 2000 |      |  3000 |      |      |      |  6000 |      | 5000 |        |       |
//     +-------+--+---+------+----+--+------+------+------+----+--+------+---+--+--------+-------+
//                |               |                            |             |
//                |               |   +------+    +-+-+-+--+   |   +-----+   |   +------+
//                |  +-+-+-+--+   +-->| 4000 |--->|d|e|f|\0|   +-->|-1368|   +-->|   4  |
//                +->|a|b|c|\0|  3000 |      |    +-+-+-+--+   6000|     |  5000 |      |
//                   +-+-+-+--+       +------+  4000 1 2 3         +-----+       +------+
//                 2000 1 2 3    3004 |      |                              5004 | 1287 |
//                                    +------+    +-+-+-+-+--+                   +------+
//                               3008 |      |--->|g| |h|i|\0|              5008 | -98  |
//                                    +------+    +-+-+-+-+--+                   +------+
//                                       :                                  5012 |   0  |
//                                    +------+                                   +------+
//                               3068 |  \0  |                              5016 |  -7  |
//                                    +------+                                   +------+
//
//
//

#include "lists.h"
#include "__rdargs.h"
#include "dosbase_private.h"

//code
#if 0
int main()
{
	UINT8 opt[200];

	//KPrintF("enter option: syntax 'Option[/S]|[/S][/A]|[/K][/M][/N][/A],...' without quotes.\n");
	//KPrintF("Two multi-arg option shall be separated by a keyword-arg option.\n\n");
	gets((char*)opt);

	int comma_count=0;
	UINT8* p = &opt[0];
	while(*p != 0)
	{
		if(*p == ',')
		{
			comma_count++;
		}
		p++;
	}

	int num_opts = comma_count + 1;
	//KPrintF("num options = %d\n", num_opts);

	UINT32* res = (UINT32*) AllocVec(sizeof(UINT32*) * num_opts, MEMF_PUBLIC);

	//we need to send only template or opts
	//and an array of longs, one for each opt
	//to store result there.
	struct RdArgs* ret;
	ret = rdargs(opt, res);
	freeargs(ret);

	return 0;
}
#endif

static UINT8* read_input(pDOSBase DOSBase, UINT8* opt);
static UINT32* alloc_mem(pDOSBase DOSBase);
static BOOL create_opt_list(pDOSBase DOSBase, UINT8* opt, UINT32* res, struct data* d);
static BOOL create_arg_list(pDOSBase DOSBase, UINT8* arg, struct data* d);
static BOOL mark_keywords(pDOSBase DOSBase, struct data* d);
static BOOL mark_keyword_for_opt(pDOSBase DOSBase, struct opt_node* opt, struct data* d);
static BOOL collect_args_with_keyword(pDOSBase DOSBase, struct data* d);
static BOOL consume_one_arg(pDOSBase DOSBase, struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword);
static BOOL consume_multiple_arg(pDOSBase DOSBase, struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword, struct data* d);
static BOOL confirm_no_extra_args(pDOSBase DOSBase, struct data* d);
static BOOL adjust_string_args(pDOSBase DOSBase, struct data* d);
static void steal_one_string_arg(pDOSBase DOSBase, struct opt_node* prev_opt, struct opt_node* opt);
static void consume_arg(UINT32* place, UINT8* arg_start);
static UINT8 count_string_args(struct opt_node* opt);
static BOOL unquote_string_args(pDOSBase DOSBase, struct data* d);
static BOOL save_result(struct data* d);
static BOOL equal(pDOSBase DOSBase, UINT8* src1_start, UINT8* src1_end, UINT8* src2_start, UINT8* src2_end);
static void curr_opt(UINT8* start, UINT8** curr_opt_start, UINT8**curr_opt_end);
static UINT8* getnext_modifier(UINT8* curr_modf);
static void next_opt(UINT8* start, UINT8** next_opt_start, UINT8**next_opt_end);
static void curr_arg(UINT8* start, UINT8** curr_arg_start, UINT8**curr_arg_end);
static BOOL convert_to_long(pDOSBase DOSBase, UINT8* p_digit, INT32* p_val);
static BOOL check_set_opts(pDOSBase DOSBase, struct data* d);
static BOOL collect_args_without_keyword(pDOSBase DOSBase, struct data* d);
static BOOL convert_numeric_string_args(pDOSBase DOSBase, struct data* d);
static BOOL check_mandatory_opts(pDOSBase DOSBase, struct data* d);
static BOOL unquote_arg(pDOSBase DOSBase, UINT32* p_arg);

void print_result(pDOSBase DOSBase, struct data* d);
void print_opt_list(pDOSBase DOSBase, struct data* d);
void print_arg_list(pDOSBase DOSBase, struct data* d);

//#undef SysBase
//extern pSysBase g_SysBase;
//#define SysBase g_SysBase

//lets start our job.
struct RdArgs* dos_ReadArgs(pDOSBase DOSBase, UINT8* opt, UINT32* res)
{
    struct data d;

    //****************************
    // read input arguments
    //****************************

    UINT8* arg;
    arg  = read_input(DOSBase, opt);
    if(arg == NULL)
    {
        return NULL;
    }

    //****************************
    // allocate memory for multi-opts
    //****************************

	UINT32* mem;
	mem = alloc_mem(DOSBase);
	if(mem == NULL)
	{
		SetIoErr(ERROR_NO_FREE_STORE);
		FreeVec(arg);
		return NULL;
	}
	else
	{
		d.all_mem = mem;
		d.free_mem = mem;
	}

	//****************************
    // allocate RdArgs to return
    //****************************

	struct RdArgs* ret;
	ret = (struct RdArgs*)AllocVec(sizeof(struct RdArgs), MEMF_PUBLIC);
	if(ret == NULL)
	{
		SetIoErr(ERROR_NO_FREE_STORE);
		FreeVec(arg);
		FreeVec(mem);
		return NULL;
	}

    //****************************
	// create list of opts
    //****************************

	if(create_opt_list(DOSBase, opt, res, &d) == FALSE)
	{
        FreeVec(arg);
        FreeVec(mem);
        FreeVec(ret);
        return NULL;
	}

    //****************************
	//create list of individual args
    //****************************

	if(create_arg_list(DOSBase, arg, &d) == FALSE)
	{
        FreeVec(arg);
        FreeVec(mem);
        FreeVec(ret);
        return NULL;
	}

	//print given things first
    //print_opt_list(DOSBase, &d);
    //print_arg_list(DOSBase, &d);

    //****************************
	// First get rid of set opts
	//****************************

	if(check_set_opts(DOSBase, &d) == TRUE

	//****************************
	// mark keyword args
	//****************************

	&& mark_keywords(DOSBase, &d) == TRUE

	//****************************
	// Now collect args for keyword opts
	// both single valued and multi valued
	//****************************

	&& collect_args_with_keyword(DOSBase, &d) == TRUE


	//****************************
	// Now collect args for non keyword opts
	// both single valued and multi valued
	//****************************

    && collect_args_without_keyword(DOSBase, &d) == TRUE

    //****************************
    // unquote quoted string args
    //****************************

    && unquote_string_args(DOSBase, &d) == TRUE

    //****************************
	// Now collect args for non keyword opts
	// both single valued and multi valued
	//****************************

    && confirm_no_extra_args(DOSBase, &d) == TRUE

	//****************************
	// Now we should donate args to unprocessed opts
	// only to single valued mandatory opts
	// for which no keyword entered by user
	//****************************

    && adjust_string_args(DOSBase, &d) == TRUE

    //****************************
    // check if we have args for mandatory opts
    //****************************

    && check_mandatory_opts(DOSBase, &d) == TRUE


    //****************************
    // convert numeric args
    //****************************

    && convert_numeric_string_args(DOSBase, &d) == TRUE

	//****************************
    // save result
    //****************************

    && save_result(&d) == TRUE)
    {

        //****************************
        // printing
        //****************************

        //print_opt_list(DOSBase, &d);
        //print_arg_list(DOSBase, &d);

        //print result only when rdargs succeeds
        //print_result(DOSBase, &d);

        //****************************
        // free memory
        //****************************

        FreeVec(d.opt_mem);
        FreeVec(d.arg_mem);

		//freeargs() will do rest
        //FreeVec(arg);
        //FreeVec(mem);
        //FreeVec(ret);

        //KPrintF("rdargs succeed\n");

        ret->inputBuf = arg;
        ret->memBuf = mem;
        return ret;
	}
    else
    {

        //****************************
        // printing
        //****************************

        //print_opt_list(&d);
        //print_arg_list(&d);

        //****************************
        // free memory
        //****************************

        FreeVec(d.opt_mem);
        FreeVec(d.arg_mem);

        FreeVec(arg);
        FreeVec(mem);
        FreeVec(ret);

        //KPrintF("rdargs failed\n");
        return NULL;
	}
	return NULL;
}

//frees memory
void dos_FreeArgs(pDOSBase DOSBase, struct RdArgs* ret)
{
	if(ret)
	{
		FreeVec(ret->inputBuf);
		FreeVec(ret->memBuf);
		FreeVec(ret);
	}
}

//to start with, we will read the input from stdin
//one char by one char till we reach a '\n'.
//plan 1:
//initially we will allocate a buffer of 256 byte,
//if we see we are filled this and we need more place,
//we will allocate 512 byte buffer, copy the old 256 collected chars to this
//and proceed. In similar fashion we will go on increasing buffer size.
//plan 2:
//initially we will allocate a buffer of 512 byte,
//which is maximum size of a command from shell.
//so we need not allocate more memory in any case.
//here we chose to implement plan 2.
//finally we put a '\0' in the place of '\n'.
static UINT8* read_input(pDOSBase DOSBase, UINT8* opt)
{
    UINT8* arg;
    arg = (UINT8*)AllocVec(BUF_CHUNK, MEMF_PUBLIC);
    if(arg == NULL)
    {
		SetIoErr(ERROR_NO_FREE_STORE);
        //KPrintF("Error: not enough memory for arg\n");
	    return NULL;
    }

	//KPrintF("\nenter args: example 'foo bar this that' without quotes.\n\n");
	FGets(Input(), arg, BUF_CHUNK-1);
	//KPrintF("\nentered args: %s\n", arg);

	//consume white spaces and TABS (che)
    UINT8* c = arg;
	while(*c == ' ' || *c == '\t')
    {
		c++;
	}

	//check if a question mark given
	if(*c == '?')
	{
		FPuts(Output(), opt);
		FPuts(Output(), ": ");
		Flush(Output());
		//KPrintF("Question Mark: \n%s\n", opt);
		FGets(Input(), arg, BUF_CHUNK-1);
	}

	//now, simply change last '\n' to '\0'.
	c = arg;
	while(*c != '\n')
	{
		c++;
	}
	*c = '\0';

    return arg;
}

//lets allocate some memory for multi-opts, so that we need not allocate memory
//every time. this will improve fastness of rdargs. for a single multi-opt we need
//an array of pointers (which points to several args) and a place for NULL pointer.
//so, when we can have MAX_ARGS args, we can allocate MAX_ARGS + MAX_ARGS pointers.

static UINT32* alloc_mem(pDOSBase DOSBase)
{
    UINT32* mem;

    //since we can have maximum MAX_ARGS arguments,
    //to be safe we should allocate MAX_ARGS + MAX_ARGS pointers
    //which includes MAX_ARGS NULL pointers.
    mem = (UINT32*)AllocVec(sizeof(UINT32*) * (MAX_ARGS + MAX_ARGS), MEMF_PUBLIC);
    if(mem == NULL)
    {
		SetIoErr(ERROR_NO_FREE_STORE);
        //KPrintF("Error: not enough memory for mem\n");
	    return NULL;
    }

	return mem;
}

//now lets create a linked list of given opts.
//besides keeping the start and end address of each opt in opt node,
//we collect the type of opt, seeing its modifiers.
//also we keep a flag to know if the opt is processed already or not.
//also we keep where to store the result of the opt.
//and more importantly we should keep track of starting position of args
//for the opt.

//this linked list is implemented on an allocated fixed array.
//to not waste time on mallocs and breaking memory into smaller fragments.

static BOOL create_opt_list(pDOSBase DOSBase, UINT8* opt, UINT32* res, struct data* d)
{
	//initialize opt_list
	NewList((pList)&d->opt_list);

	UINT8* curr_opt_start = 0;
	UINT8* curr_opt_end = 0;

	UINT8* next_opt_start = 0;
	UINT8* next_opt_end = 0;

	UINT8* opt_start = opt;
	UINT32* res_start = res;

	d->opt_mem = (struct opt_node*)AllocVec(MAX_OPTS * sizeof(struct opt_node), MEMF_PUBLIC);

	if(d->opt_mem == NULL)
	{
	    KPrintF("Error: not enough memory for opt structures\n");
		SetIoErr(ERROR_NO_FREE_STORE);
	    return FALSE;
	}

    struct opt_node* new_opt = 0;
    new_opt = d->opt_mem;

	while(opt_start != NULL)
	{
		curr_opt(opt_start, &curr_opt_start, &curr_opt_end);
		next_opt(curr_opt_start, &next_opt_start, &next_opt_end);

		if(curr_opt_start != NULL)
		{
		    if(new_opt > (d->opt_mem + (MAX_OPTS-1)))
		    {
		        KPrintF("Error: too many opts given\n");
		        FreeVec(d->opt_mem);
				SetIoErr(ERROR_TOO_MANY_ARGS);
		        return FALSE;
		    }

            //add an opt node
			AddTail((pList)&d->opt_list, (pNode)&new_opt->o_node);

			new_opt->o_start = curr_opt_start;
			new_opt->o_end = curr_opt_end;
			new_opt->o_modf = 0;
			new_opt->o_processed = 0;
			new_opt->o_has_result = 0;
			new_opt->o_res = res_start;
			//initialize temp result to "no result";
			new_opt->o_temp_res = 0;
			//the above initialization is only made to temp res,
			//for caller might have set a default value for opt
			//in res array, pointed by o_res.
			new_opt->o_arg_start = 0;

			UINT8* p = getnext_modifier(curr_opt_end);
			while(p != NULL)
			{
				switch(*p)
				{
					case 'S': new_opt->o_modf |= MODF_S; break;
					case 'K': new_opt->o_modf |= MODF_K; break;
					case 'M': new_opt->o_modf |= MODF_M; break;
					case 'N': new_opt->o_modf |= MODF_N; break;
					case 'A': new_opt->o_modf |= MODF_A; break;
					default:
                        KPrintF("Bad Modifier %c\n",*p);
						SetIoErr(ERROR_BAD_NUMBER);
                        FreeVec(d->opt_mem);
                        return FALSE;
				}
				p = getnext_modifier(p);
			}
		}

		opt_start = next_opt_start;
		res_start++;
		new_opt++;
	}

	if(IsListEmpty((pList)&d->opt_list))
	{
		KPrintF("Empty opt list\n");
		//SetIoErr(ERROR_BAD_NUMBER);
        FreeVec(d->opt_mem);
		return FALSE;
	}
	return TRUE;
}

//now lets create a linked list of given args.
//besides keeping the start and end address of each arg in arg node,
//we collect the type of arg, if it is keyword or value.

//this linked list is implemented on an allocated fixed array.
//to not waste time on mallocs and breaking memory into smaller fragments.

static BOOL create_arg_list(pDOSBase DOSBase, UINT8* arg, struct data* d)
{
	//initialize arg_list
	NewList((pList) &d->arg_list);

	UINT8* curr_arg_start = 0;
	UINT8* curr_arg_end = 0;

	UINT8* arg_start = arg;

	d->arg_mem = (struct arg_node*)AllocVec(MAX_ARGS * sizeof(struct arg_node), MEMF_PUBLIC);

	if(d->arg_mem == NULL)
	{
	    //KPrintF("Error: not enough memory for arg structures\n");
		SetIoErr(ERROR_NO_FREE_STORE);
	    return FALSE;
	}

    struct arg_node* new_arg = 0;
    new_arg = d->arg_mem;

	while(arg_start != NULL)
	{
		curr_arg(arg_start, &curr_arg_start, &curr_arg_end);

		if(curr_arg_start != NULL)
		{
			if(new_arg > (d->arg_mem + (MAX_ARGS-1)))
		    {
		        //KPrintF("Error: too many args given\n");
				SetIoErr(ERROR_TOO_MANY_ARGS);
		        FreeVec(d->arg_mem);
		        return FALSE;
		    }

            //add an arg node
			AddTail((pList)&d->arg_list, (pNode)&new_arg->a_node);

			new_arg->a_start = curr_arg_start;
			new_arg->a_end = curr_arg_end;
			new_arg->a_keyword = 0;

			arg_start = curr_arg_end+1;
		}
		else
		{
            arg_start = NULL;
		}

		new_arg++;
	}

	struct MinNode *a_node, *a;
    //check all arg
    ForeachNodeSafe(&d->arg_list, a_node, a)
    {
        struct arg_node* arg = (struct arg_node*)a_node;

		//add a '\0' to the end of each arg
		UINT8* p = arg->a_end;
		p++;
		if((*p == ' ')
		|| (*p == '\t')
		|| (*p == '=')
		|| (*p == '\0'))
		{
			*p = '\0';
		}
		else
		{
            //KPrintF("Error: args should be separated by space or equal sign\n");
			SetIoErr(ERROR_REQUIRED_ARG_MISSING);
            FreeVec(d->arg_mem);
			return FALSE;
		}
    }
	return TRUE;
}

//now we have a list of opts and we have a list of args
//here, what we have to do is, for each opt,

//if it a S type opt,
//go through all args and see if it appears in arg list, if that opt is found,
//set the result and mark that opt as processed.
//also we will remove that arg from arg list, so that it won't trouble us in future.


static BOOL check_set_opts(pDOSBase DOSBase, struct data* d)
{
    struct MinNode *o_node, *o;
	struct MinNode *a_node, *a;

    //do for each opt
    ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//if it is S opt
		if(TEST_BITS(opt->o_modf, MODF_S))
		{
			//check all arg
			ForeachNodeSafe(&d->arg_list, a_node, a)
			{
				struct arg_node* arg = (struct arg_node*)a_node;
				//if matched arg found
				if(equal(DOSBase, opt->o_start, opt->o_end, arg->a_start, arg->a_end) == TRUE)
				{
					//set 1 in result
					opt->o_temp_res = 1;

					//say it got some value
					opt->o_has_result = 1;

					//delete that arg
					Remove((pNode)a_node);
				}
				else
				{
					continue;
				}
			}

			//mark this opt as processed
			opt->o_processed = 1;
		}
    }

    return TRUE;
}


//now for each unprocessed opts, check

//if it is a K opt,
//and it is found in arg list, save the position of that arg with opt
//and mark that arg as a keyword
//(we will come back to this arg position later for further processing of the opt.)
//if that opt is not found in arg list, we know that this opt can't have any value,
//so mark the opt as processed.

//if it is not a K opt,
//even then chances are there that the user might have passed the opt name in the arg list!
//so, we treat and process this opt similar to K opt
//but only thing is that if that opt is not found in arg list,
//we don't mark the opt as processed.
static BOOL mark_keywords(pDOSBase DOSBase, struct data* d)
{
    struct MinNode *o_node, *o;

    //do for each opt
    ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;

        //when they are not processed yet
		if(opt->o_processed == 0)
		{
            //mark the keyword args for this opt;
            //for K opt as well as non K opt
            if(mark_keyword_for_opt(DOSBase, opt, d) == FALSE)
            {
                return FALSE;
            }

			//if it is K opt
			if(TEST_BITS(opt->o_modf, MODF_K))
			{
				//if no matched arg found
				if(opt->o_arg_start == 0)
				{
                    //say we didn't get any input for this
                    opt->o_has_result = 0;
					//mark this opt as processed
					opt->o_processed = 1;
				}

			}
			//if it is non K opt
			else
			{
				//no harm
			}
		}
	}

    return TRUE;
}

static BOOL mark_keyword_for_opt(pDOSBase DOSBase, struct opt_node* opt, struct data* d)
{
	struct MinNode *a_node, *a;
    //check all arg
    ForeachNodeSafe(&d->arg_list, a_node, a)
    {
        struct arg_node* arg = (struct arg_node*)a_node;
        //if matched arg found
        if(equal(DOSBase, opt->o_start, opt->o_end, arg->a_start, arg->a_end) == TRUE)
        {
            //save position of arg with opt
            if(opt->o_arg_start == 0)
            {
                opt->o_arg_start = arg;
            }
            else
            {
                //Error
                //KPrintF("Error: a keyword found multiple times\n");
				SetIoErr(ERROR_TOO_MANY_ARGS);
                return FALSE;
            }
            //mark that arg as keyword
            arg->a_keyword = 1;
        }
        else
        {
            continue;
        }
    }

    return TRUE;
}

//now we will collect the args with a keyword preceeding.
//remember that both for K opts and non-K opts, keyword can be passed in argument.
//hence we have already identified if we have keywords in the arg list
//and we have collected the start position of the args for such keyword preceeding args.
//so we check all not-processed opts...if they have arg position with them, we attack.
//we consume the value args along with the keyword arg.
//Actually keyword arg is simply removed from the arg list.
//value args are collected as part of the result of the opt, and are removed from arg list.
//And finally we mark the opt processed.
static BOOL collect_args_with_keyword(pDOSBase DOSBase, struct data* d)
{
   	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//non processed opt
		if(opt->o_processed == 0)
		{
			//and when opt keyword was found in arg
			if(opt->o_arg_start != 0)
			{
				//if it is a single valued opt
				if(!TEST_BITS(opt->o_modf, MODF_M))
				{
					struct arg_node* next_arg = GetSucc(opt->o_arg_start);

                    //if we have consumable arg next to found position
                    if(next_arg != NULL )
                    {
                        //consume it
                        if(consume_one_arg(DOSBase, next_arg, opt, TRUE) == TRUE)
                        {
                            //delete keyword arg
                            Remove((pNode)&opt->o_arg_start->a_node);
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                    //if we don't have consumable arg next to found position
                    else
                    {
                        //Error
                        //KPrintF("Error: no arguments found for a option\n");
						SetIoErr(ERROR_KEY_NEEDS_ARG);
                        return FALSE;
                    }
				}
				//if multi valued opt
				else
				{
					struct arg_node* next_arg = GetSucc(opt->o_arg_start);

                    //if we have consumable arg next to found position
                    if(next_arg != NULL )
                    {
                        //consume as many as possible
                        if(consume_multiple_arg(DOSBase, next_arg, opt, TRUE, d) == TRUE)
                        {
                            //delete keyword arg
                            Remove((pNode)&opt->o_arg_start->a_node);
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                    //if we don't have consumable arg next to found position
                    else
                    {
                        //Error
                        //KPrintF("Error: no arguments found for a option\n");
						SetIoErr(ERROR_KEY_NEEDS_ARG);
                        return FALSE;
                    }
				}
			}
		}
	}

    return TRUE;
}

//consumes only one arg that is not a keyword
static BOOL consume_one_arg(pDOSBase DOSBase, struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword)
{
    //and if we can consume this arg
    if(next_arg->a_keyword == 0)
    {
        //consume it
        UINT32* res = 0;

        res = &opt->o_temp_res;
        consume_arg( res, next_arg->a_start );
        //KPrintF("opt->o_temp_res = %#x\n", opt->o_temp_res);

        //delete consumed arg
        Remove((pNode)&next_arg->a_node);

        //say we got input
        opt->o_has_result = 1;

        //mark opt as processed
        opt->o_processed = 1;
    }
    //lo, we can't consume this arg
    else
    {
		if(after_keyword)
		{
        	//Error
        	//KPrintF("Error: no arguments found for a option\n");
			SetIoErr(ERROR_KEY_NEEDS_ARG);
        	return FALSE;
    	}
		else
		{
			//no issues
		}
    }

    return TRUE;
}

//goes on consuming args till it finds a keyword
static BOOL consume_multiple_arg(pDOSBase DOSBase, struct arg_node* next_arg, struct opt_node* opt, BOOL after_keyword, struct data* d)
{

    struct arg_node* temp = next_arg;

    //and if we can consume this arg
    if(next_arg->a_keyword == 0)
    {
        //consume args as many as possible
        UINT8 num_args = 0;
        while(temp != NULL && temp->a_keyword == 0)
        {
            num_args++;
            temp = GetSucc(temp);
        }

        UINT32* res = 0;
        //opt->o_temp_res = (UINT32)AllocVec(sizeof(UINT32*) * (num_args+1), MEMF_PUBLIC);
        //lets not malloc, but use chunks from previously allocated memory
        opt->o_temp_res = (UINT32)d->free_mem;
        d->free_mem += (num_args+1);

        //KPrintF("no-malloc opt->o_temp_res = %#x\n", opt->o_temp_res);

        res = (UINT32*) opt->o_temp_res;
        UINT8 i;
        for(i = 0; i < num_args; i++)
        {
            consume_arg( (res+i), next_arg->a_start );
            struct arg_node* temp = next_arg;
            next_arg = GetSucc(next_arg);

            //delete consumed arg
            Remove((pNode)&temp->a_node);
        }
        *(res+i) = '\0';

        //say we got input
        opt->o_has_result = 1;

        //mark opt as processed
        opt->o_processed = 1;
    }
    //lo, we can't consume this arg
    else
    {
		if(after_keyword)
    	{
        	//Error
        	//KPrintF("Error: no arguments found for a option\n");
			SetIoErr(ERROR_KEY_NEEDS_ARG);
        	return FALSE;
    	}
		else
		{
			//no issues
		}
    }

    return TRUE;
}


//now time to attack args without a keyword preceeding.
//here we have to consume args from left to right as per position of opts in template.
//so for each unprocessed opt, if it is single valued opt,
//we consume one arg, mean, we collect the arg as part of the result of the opt,
//and remove that arg from arg list. finally we mark the opt as processed.
//same for multivalued opt as well.
static BOOL collect_args_without_keyword(pDOSBase DOSBase, struct data* d)
{
   	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//non processed opt
		if(opt->o_processed == 0)
		{
			//and when opt word was not found in arg
			if(opt->o_arg_start == 0)
			{
				//if it is a single valued opt
				if(!TEST_BITS(opt->o_modf, MODF_M))
				{
					struct arg_node* next_arg = (struct arg_node*)GetHead(&d->arg_list);
					//if we have consumable arg
					if(next_arg != NULL )
					{
                        if(consume_one_arg(DOSBase, next_arg, opt, FALSE) == FALSE)
                        {
                            return FALSE;
                        }
					}
					//if we don't have consumable arg
					else
					{
						//we just give a warning...
						//an opt may not have any args at all...
						//for mandatory opts, this could be an error
						//but we check mandatory opt cases later and report error.
						//for optional opts, this is not an error

						//when an opt is a single-valued-opt,
						//may be it is trying to steal some arg from others
						//so, lets keep the opt as un-processed and wait little more
						//KPrintF("Warning: no arguments found for an option, it may steal args from others\n");
					}
				}
				//if multi valued opt
				else
				{
					struct arg_node* next_arg = (struct arg_node*)GetHead(&d->arg_list);
					//if we have consumable arg
					if(next_arg != NULL )
					{
                        //consume as many as possible
                        if(consume_multiple_arg(DOSBase, next_arg, opt, FALSE, d) == FALSE)
                        {
                            return FALSE;
                        }
					}
					else
					{
						//we just give a warning...
						//an opt may not have any args at all...
						//for mandatory opts, this could be an error
						//but we check mandatory opt cases later and report error.
						//for optional opts, this is not an error
						//KPrintF("Warning: no arguments found for an option\n");
					}
				}
			}
		}
	}

    return TRUE;
}


static BOOL confirm_no_extra_args(pDOSBase DOSBase, struct data* d)
{
    if(!IsListEmpty(&d->arg_list))
    {
		int i =0;
		struct arg_node *tmp;

		ForeachNode(&d->arg_list, tmp)
		{
			i++;
			UINT8* cnt = tmp->a_start;
			//KPrintF("a_start: %x a_end: %x\n", tmp->a_start, tmp->a_end);
			//while(cnt <= tmp->a_end) KPrintF("%x,", *cnt++);
			//KPrintF("\n");
		}
		//KPrintF("Error: extra args found: %d\n",i);
		SetIoErr(ERROR_TOO_MANY_ARGS);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//now, time to adjust the args! there could be still some unprocessed opts left!
//we can have cases like, a single valued opt preceeded with a multiple valued opt.
//if args for this single valued opt doesn't come with a keyword preceeding,
//the args meant for this opt shall obviously be consumed by preceeding multivalued opt.
//we have to steal the arg that was meant for single valued opt.
static BOOL adjust_string_args(pDOSBase DOSBase, struct data* d)
{

   	struct MinNode *o_node, *o;

   	//do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//non processed opt
		if(opt->o_processed == 0)
		{
			//if below are the status of current opt,
			//only when we will proceed to steal an arg from previous opt

			//if opt word was not found in arg
			//and single valued
			//and mandatory
			if(opt->o_arg_start == 0
			&& !TEST_BITS(opt->o_modf, MODF_M)
			&& TEST_BITS(opt->o_modf, MODF_A))
			{

				struct opt_node* prev_opt = GetPred(opt);

				if(prev_opt != NULL)
				{

					//if previous opt is processed
					//and has some input as its result
					//and previous opt was not found in arg
					//(Important: if previous opt was keyworded, we never touch it)
					if(prev_opt->o_processed == 1
					&& prev_opt->o_has_result == 1
					&& prev_opt->o_arg_start == 0)
					{
						//now we have to see what should be the status of the previous
						//option, so that we can safely steal an argument from it.

						//N = number of argument alloted to previous opt
						//P = status of previous opt
						//C = status of current opt
						//D = decision

						//m = mandatory
						//o = not-mandatory(optional)

						//+ = steal
						//- = don't steal

						//previous opt is multi-valued
						//N P C D
						//=======
						//1 o o -
						//1 o m +
						//1 m o -
						//1 m m -

						//previous opt is multi-valued
						//N P C D
						//=======
						//2 o o -
						//2 o m +
						//2 m o -
						//2 m m +

						//previous opt is single-valued
						//N P C D
						//=======
						//1 o o -
						//1 o m +
						//1 m o -
						//1 m m -

						//TODO: some of the above descisions may be changed,
						//to allow setting of default values in res array
						//for single-valued, optional, numeric opts

						UINT8 num_args = count_string_args(prev_opt);

						if(

						(TEST_BITS(prev_opt->o_modf, MODF_M)
						&& num_args == 1
						&& !TEST_BITS(prev_opt->o_modf, MODF_A))

						||

						(TEST_BITS(prev_opt->o_modf, MODF_M)
						&& num_args > 1
						&& !TEST_BITS(prev_opt->o_modf, MODF_A))

						||

						(TEST_BITS(prev_opt->o_modf, MODF_M)
						&& num_args > 1
						&& TEST_BITS(prev_opt->o_modf, MODF_A))

						||

						(!TEST_BITS(prev_opt->o_modf, MODF_M)
						&& num_args == 1
						&& !TEST_BITS(prev_opt->o_modf, MODF_A))

						)
						{
							//iff type of both opts same
							if((TEST_BITS(opt->o_modf, MODF_N)
							&& TEST_BITS(prev_opt->o_modf, MODF_N))
							|| (!TEST_BITS(opt->o_modf, MODF_N)
							&& !TEST_BITS(prev_opt->o_modf, MODF_N))
							)
							{
								//steal one arg
								steal_one_string_arg(DOSBase, prev_opt, opt);
							}
							else
							{
								//we just give a warning...
								//an opt may not have any args at all...
								//for optional opt, we should give a warning here,
								//however, this function is not for optional opts!
								//for mandatory opts, we should give an error here,
								//but we check mandatory opt cases later and report error.
								//so, lets just return with an warning.
								//after all, the function name suggests, it is an optional step
								//to sreal an argument, if we have an opprtunity.
								//KPrintF("Warning: no arguments found for an option, couldn't even steal from others\n");
							}
						}
						else
						{
							//see the reason above.
							//KPrintF("Warning: no arguments found for an option, couldn't even steal from others\n");
						}
					}
					else
					{
						//see the reason above.
						//KPrintF("Warning: no arguments found for an option, couldn't even steal from others\n");
					}
				}
				else
				{
					//see the reason above.
					//KPrintF("Warning: no arguments found for an option, couldn't even steal from others\n");
				}
			}
		}
	}

	return TRUE;
}

//stealing one arg is easy!
//go to the result arg set of the previous opt,
//locate the last arg, remove it from the result set of previous opt
//and bring it to the result set of current opt.
static void steal_one_string_arg(pDOSBase DOSBase, struct opt_node* prev_opt, struct opt_node* opt)
{
	UINT32* prev_opt_res = (UINT32*) prev_opt->o_temp_res;

    if(prev_opt->o_processed == 1
    && prev_opt->o_has_result == 1)
    {
        if(prev_opt_res == NULL)
        {
            return;
        }
        else if((UINT32)prev_opt_res == (UINT32)1)
        {
            //this should never happen, but still in case we are unlucky
            return;
        }
        else
        {
            //if prev_opt has multiple args
            if(count_string_args(prev_opt) > 1)
            {
                //just bring last arg from prev_opt
                UINT32* p = prev_opt_res;
                while(*p != 0)
                {
                    p++;
                }
                p--;

                //we got the last arg pointed by p
                UINT8* arg_start = (UINT8*)*p;

                //now steal it
                UINT32* res = &opt->o_temp_res;
                consume_arg( res, arg_start );

                //say we got input
                opt->o_has_result = 1;

                //mark opt as processed
                opt->o_processed = 1;

                //and make the stolen arg pointer null in prev_opt res
                *p = 0;
            }
            //if prev_opt has one arg only
            else
            {
                //if it is a multivalued opt
                if(TEST_BITS(prev_opt->o_modf, MODF_M))
                {
                    //bring arg from prev_opt
                    UINT32* p = prev_opt_res;
                    while(*p != 0)
                    {
                        p++;
                    }
                    p--;

                    //we got the last arg pointed by p
                    UINT8* arg_start = (UINT8*)*p;

                    //now steal it
                    UINT32* res = &opt->o_temp_res;
                    consume_arg( res, arg_start );

                    //say we got input
                    opt->o_has_result = 1;

                    //mark opt as processed
                    opt->o_processed = 1;

                    //say we have no input for previous opt
                    prev_opt->o_has_result = 0;

                    //and make the stolen arg pointer null in prev_opt res
                    *p = 0;

                    //free prev_opt res
                    //but now we dont need to free anything.
                    //freearg() will take care of it.
                    //FreeVec((UINT32*) prev_opt->o_temp_res);

                    //make prev_opt res null
                    prev_opt->o_temp_res = 0;
                }
                //else if it is a single valued opt
                else
                {
                    //bring arg from prev_opt
                    UINT32* p = prev_opt_res;

                    //we got the last arg pointed by p
                    UINT8* arg_start = (UINT8*)p;

                    //now steal it
                    UINT32* res = &opt->o_temp_res;
                    consume_arg( res, arg_start );

                    //say we got input
                    opt->o_has_result = 1;

                    //mark opt as processed
                    opt->o_processed = 1;

                    //say we have no input for previous opt
                    prev_opt->o_has_result = 0;

                    //make prev_opt res null
                    prev_opt->o_temp_res = 0;
                }
            }
        }
	}
}

//consume current arg, given start address of the arg.
//here earlier we decided to malloc a memory chunck and copy the arg to it.
//But later on we thought, malloc shall be time consuming,
//so now we simply point to the start address of the arg
static void consume_arg(UINT32* place, UINT8* arg_start)
{
	*place = (UINT32)arg_start;
	//KPrintF("place = %#x\n", place);
    //KPrintF("*place = %#x\n", *place);
}


//count the number of args present in result set of an opt.
static UINT8 count_string_args(struct opt_node* opt)
{
	UINT32* opt_res = (UINT32*) opt->o_temp_res;
	UINT8 numargs = 0;
	if(opt->o_processed == 1
	&& opt->o_has_result == 1)
	{
        if(opt_res == NULL)
        {
            //when such an odd thing can happen?
            //opt is processed and has result yet result is zero!
            //only for a numeric opt.
            //but here we are calculating number of string args.
            //so return 0.
            numargs = 0;
        }
        else if((UINT32)opt_res == (UINT32)1)
        {
            //this should never happen
            //kept only for S opts!
            numargs = 1;
        }
        else
        {
            //now we have a valid address

            //if opt is of multivalued
            if(TEST_BITS(opt->o_modf, MODF_M))
            {
                //this address points to an array of args
                //go on counting number of args it has
                UINT32* p = opt_res;
                while(*p != 0)
                {
                    numargs++;
                    p++;
                }
            }
            //if opt is of single valued
            else
            {
                //this address points to the arg itself
                //say that we have one arg
                numargs = 1;
            }
        }
    }
    else
    {
        numargs = 0;
    }

	return numargs;
}


//check if we got args for mandatory opts
//else, fire!
static BOOL check_mandatory_opts(pDOSBase DOSBase, struct data* d)
{
   	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//processed opt
		if(TEST_BITS(opt->o_modf, MODF_A))
		{
            if(!(opt->o_processed == 1 && opt->o_has_result == 1))
            {
                //KPrintF("Error: No args found for a mandatory opt\n");
				SetIoErr(ERROR_KEY_NEEDS_ARG);
                return FALSE;
            }
		}
    }

    return TRUE;
}

//unquote args of non-numeric opts
static BOOL unquote_string_args(pDOSBase DOSBase, struct data* d)
{
   	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;

		//string opt
		if(!TEST_BITS(opt->o_modf, MODF_N))
		{
            //if processed
            //and has some args
            if(opt->o_processed == 1
            && opt->o_has_result == 1)
            {
                if(opt->o_temp_res == 0)
                {
                }
                else if(opt->o_temp_res == 1)
                {
                }
                else
                {
                    //when opt is multivalued
                    if(TEST_BITS(opt->o_modf, MODF_M))
                    {
                        UINT32* p = (UINT32*) opt->o_temp_res;
                        while(*p != 0)
                        {
                            //unquote
                            if(unquote_arg(DOSBase, p) == FALSE)
                            {
                                return FALSE;
                            }

                            p++;
                        }
                    }
                    //when opt is single valued
                    else
                    {
                        UINT32* p = &opt->o_temp_res;
                        //unquote
                        if(unquote_arg(DOSBase, p) == FALSE)
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }
    }

    return TRUE;
}

//unquote a string arg
static BOOL unquote_arg(pDOSBase DOSBase, UINT32* p_arg)
{

    UINT8* curr_arg_start = (UINT8*) *p_arg;
    UINT8* curr_arg_end = curr_arg_start;

    //check if it is a quoted arg
    //proceed to remove quotes
    if(*curr_arg_start == '\"')
    {
        //find the arg end
        while(*curr_arg_end != 0)
        {
            curr_arg_end++;
        }
        curr_arg_end--;

        //if we see a quote at end
        if(*curr_arg_end == '\"')
        {
            if(curr_arg_start == curr_arg_end)
            {
                //KPrintF("Error: a quote missing\n");
				SetIoErr(ERROR_UNMATCHED_QUOTES);
                return FALSE;
            }
            else if(curr_arg_start + 1 == curr_arg_end)
            {
                //KPrintF("Error: quoted string empty \n");
				SetIoErr(ERROR_BAD_NUMBER);
                return FALSE;
            }
            else
            {
                //now we unquoted
                curr_arg_start++;
                curr_arg_end--;
            }

            //we place a null char at the place of end-quote
            *(curr_arg_end+1) = '\0';

            //reassign unquoted string to its own place
            *p_arg = (UINT32)curr_arg_start;
        }
        else
        {
            //KPrintF("Error: a quote missing\n");
			SetIoErr(ERROR_UNMATCHED_QUOTES);
            return FALSE;
        }
    }

    return TRUE;
}

//lets convert numeric args from string to signed 32bit integer.
//when an opt is processed and numeric in nature, we attack.
//for single valued opt, we will convert and store the result in result location,
//for multivalued opt, we will do a trick.
//for multivalued opt, we know that we have an array of args in result.
//we will rearrange the array of args such that first element of the arg array
//shall hold the number of numeric args the array contains.
//then all numeric args shall be arranged one after another in this array.
static BOOL convert_numeric_string_args(pDOSBase DOSBase, struct data* d)
{
   	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;

		//numeric opt
		if(TEST_BITS(opt->o_modf, MODF_N))
		{
            //if processed
            //and has some args
            if(opt->o_processed == 1
            && opt->o_has_result == 1)
            {
                if(opt->o_temp_res == 0)
                {
                }
                else if(opt->o_temp_res == 1)
                {
                }
                else
                {
                    //when opt is multivalued
                    if(TEST_BITS(opt->o_modf, MODF_M))
                    {
                        //count numer of args
                        UINT8 num_args = count_string_args(opt);

                        //find out last arg position
                        UINT32* first_arg = (UINT32*) opt->o_temp_res;
                        UINT32* last_arg = first_arg + (num_args-1);

                        //collect the pointer to numeric arg string
                        UINT8* numeric_arg = (UINT8*) *last_arg;

                        //do for each arg
                        UINT8 i = 0;
                        while(i < num_args)
                        {
                            INT32 val;

                            //convert
                            if(convert_to_long(DOSBase, numeric_arg, &val) == FALSE)
                            {
                                return FALSE;
                            }

                            //KPrintF("%s converted to %d\n", numeric_arg, val);

                            //store it in next position
                            //(the last arg value gets stored in the place where '\0' was there)
                            *(last_arg+1) = val;

                            //come to previous arg position
                            last_arg--;

                            //collect the pointer to numeric arg string
                            numeric_arg = (UINT8*) *last_arg;

                            i++;
                        }

                        //store number of args in first element of arg array.
                        *first_arg = num_args;

                    }
                    //when opt is single valued
                    else
                    {
                        UINT8* numeric_arg = (UINT8*) opt->o_temp_res;
                        INT32 val;
                        if(convert_to_long(DOSBase, numeric_arg, &val) == FALSE)
                        {
                            return FALSE;
                        }

						//allocate memory for a INT32
                        opt->o_temp_res = (UINT32)d->free_mem;
						d->free_mem += 1;

                        //KPrintF("%s converted to %d\n", numeric_arg, val);
   						UINT32* res = (UINT32*) opt->o_temp_res;
                        *(res) = val;
                    }
                }
            }
        }
    }

    return TRUE;
}

static BOOL save_result(struct data* d)
{
	struct MinNode *o_node, *o;

    //do for each
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		//processed opt
		//and has some input
		if(opt->o_processed == 1
		&& opt->o_has_result == 1)
		{
			*(opt->o_res) = opt->o_temp_res;
		}
	}

	return TRUE;
}

//checks if two strings equal, given their start and end addresses.
static BOOL equal(pDOSBase DOSBase, UINT8* src1_start, UINT8* src1_end, UINT8* src2_start, UINT8* src2_end)
{
	int i;
	BOOL equal = TRUE;
	if(src1_start == NULL
	|| src1_end == NULL
	|| src2_start == NULL
	|| src2_end == NULL)
	{
		return FALSE;
	}

	if(src2_end-src2_start == src1_end-src1_start)
	{
		for(i=0; i < src2_end-src2_start+1; i++)
		{
			if(ToLower(*src1_start) != ToLower(*src2_start))
			{
				equal = FALSE;
			}
			src1_start++;
			src2_start++;
		}

		if(equal == FALSE)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}


//returns two valid pointers
//to current opt
//or returns two NULL pointers
static void curr_opt(UINT8* start, UINT8** curr_opt_start, UINT8**curr_opt_end)
{
	if(start == NULL || *start == '\0')
	{
		*curr_opt_start = NULL;
		*curr_opt_end = NULL;
		return;
	}
	*curr_opt_start = start;
	*curr_opt_end = start;
	while((**curr_opt_end != '/')
	&& (**curr_opt_end != ',')
	&& (**curr_opt_end != '\0'))
	{
		(*curr_opt_end)++;
	}
	(*curr_opt_end)--;
}

//returns null if
//no more modifiers found for an opt
//or '\0' encountered.
//else return the modifier.
static UINT8* getnext_modifier(UINT8* curr_modf)
{
	UINT8* p = curr_modf;
	while(*p != '/')
	{
		if(*p == '\0'
		|| *p == ',')
		{
			return NULL;
		}

		p++;
	}
	p++;
	//KPrintF("modifier = %c\n", *p);
	return p;
}

//given current option,
//returns two valid pointers
//to next opt
//or returns two NULL pointers
static void next_opt(UINT8* start, UINT8** next_opt_start, UINT8**next_opt_end)
{
	if(start == NULL || *start == '\0')
	{
		*next_opt_start = NULL;
		*next_opt_end = NULL;
		return;
	}

	*next_opt_start = start;
	while(**next_opt_start != ',')
	{
		if(**next_opt_start == '\0')
		{
			*next_opt_start = NULL;
			*next_opt_end = NULL;
			return;
		}
		(*next_opt_start)++;
	}

	(*next_opt_start)++;
	if(**next_opt_start == '\0')
    {
        *next_opt_start = NULL;
        *next_opt_end = NULL;
        return;
    }

	*next_opt_end = *next_opt_start;
	while((**next_opt_end != '/')
	&& (**next_opt_end != ',')
	&& (**next_opt_end != '\0'))
	{
		(*next_opt_end)++;
	}
	(*next_opt_end)--;
}

//returns two valid pointers
//to current arg
//or returns two NULL pointers
static void curr_arg(UINT8* start, UINT8** curr_arg_start, UINT8**curr_arg_end)
{
	if(start == NULL || *start == '\0')
	{
		*curr_arg_start = NULL;
		*curr_arg_end = NULL;
		return;
	}

	*curr_arg_start = start;
	//consume spaces
	//and equal signs
	// and TABS !!! (che)
	while((**curr_arg_start == ' ')
	|| (**curr_arg_start == '=')
	|| (**curr_arg_start == '\t'))
	{
		(*curr_arg_start)++;
	}

	if(**curr_arg_start == '\0')
    {
        *curr_arg_start = NULL;
        *curr_arg_end = NULL;
        return;
    }

    *curr_arg_end = *curr_arg_start;

    //in case we get a quote
	if(**curr_arg_start == '\"')
	{
        (*curr_arg_end)++;

        //argument spans till next quote
        while((**curr_arg_end != '\"')
        && (**curr_arg_end != '\0'))
        {
            (*curr_arg_end)++;
        }
        if(**curr_arg_end == '\0')
        {
            (*curr_arg_end)--;
        }
	}
	else
	{
        //argument spans till next space
        //or equal sign
		// or Tab?  (CHE)
        while((**curr_arg_end != ' ')
        && (**curr_arg_end != '=')
        && (**curr_arg_end != '\0')
		&& (**curr_arg_end != '\t')
		)
        {
            (*curr_arg_end)++;
        }
        (*curr_arg_end)--;
	}
}

//converts a given string of digits to signed 32 bit integer
//along with plus/minus sign, only decomal numbers are converted
static BOOL convert_to_long(pDOSBase DOSBase, UINT8* p_digit, INT32* p_val)
{
    UINT8 negative = 0;
    INT32 val=0;
    INT8 len;
    UINT8* p = p_digit;

    //check if negative
    if(*p_digit == '-')
    {
        negative = 1;
        p_digit++;
    }
    else if(*p_digit == '+')
    {
        p_digit++;
    }

    //consume prefix 0's
    while(*p_digit == '0')
    {
        p_digit++;
    }

    p = p_digit;

    //process one by one digit till end of the string
    while(*p_digit != '\0')
    {
        if(*p_digit >= '0' && *p_digit <= '9')
        {
            val = val*10 + (*p_digit-'0');
        }
        else
        {
            //KPrintF("Error: Not a decimal number.\n");
            return FALSE;
        }
        p_digit++;
    }

    //check if overflow occured
    len = Strlen((STRPTR)p);
    if(len > 10)
    {
        //KPrintF("Error: Too large number.\n");
		SetIoErr(ERROR_BAD_NUMBER);
        return FALSE;
    }
    else if(len == 10)
    {
        if(negative)
        {
            if(Strcmp((STRPTR)p, "2147483648") > 0)
            {
                //KPrintF("Error: Too small number.\n");
				SetIoErr(ERROR_BAD_NUMBER);
                return FALSE;
            }
        }
        else
        {
            if(Strcmp((STRPTR)p, "2147483647") > 0)
            {
                //KPrintF("Error: Too large number.\n");
				SetIoErr(ERROR_BAD_NUMBER);
                return FALSE;
            }
        }
    }

    //when number is negative
    if(negative)
    {
        val = val * (-1);
    }

    *p_val = val;
    return TRUE;
}

//print the result stored in the array of longs that
//was sent to rdargs. Use one long per opt and interprete
//it as per nature of opt
void print_result(pDOSBase DOSBase, struct data* d)
{
    struct MinNode *o_node, *o;
    //do for each opt
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;

		//if that opt has no input
        if(opt->o_processed == 1
        && opt->o_has_result == 1)
        {
            //if opt is S
            if(TEST_BITS(opt->o_modf, MODF_S))
            {
				//if set
                if((UINT32)*opt->o_res == (UINT32)1)
                {
                    KPrintF("--- SET ");
                    KPrintF("*******\n");
                }
                else
                {
                    KPrintF("--- NOT SET ");
                    KPrintF("*******\n");
                }
            }
            else
            {
                //if opt is M
                if(TEST_BITS(opt->o_modf, MODF_M))
                {
                    //if opt is N
                    if(TEST_BITS(opt->o_modf, MODF_N))
                    {
                        UINT32* p = (UINT32*) *opt->o_res;

                        //collect number of args
                        UINT8 num_args = (UINT8)*p;

                        //go to first arg
                        p++;

                        while(num_args > 0)
                        {
                            KPrintF("--- %d\n", (INT32)*p);

                            //go to next arg
                            p++;

                            num_args--;
                        }
                        KPrintF("*******\n");
                    }
                    else
                    {
                        UINT32* p = (UINT32*) *opt->o_res;
                        while(*p != 0)
                        {
                            KPrintF("--- %s\n", (UINT8*)*p);
                            p++;
                        }
                        KPrintF("*******\n");
                    }
                }
                //if opt is single valued
                else
                {
                    //if opt is N
                    if(TEST_BITS(opt->o_modf, MODF_N))
                    {
						INT32* p = (INT32*) *opt->o_res;
                        KPrintF("--- %d ", *p);
                        KPrintF("*******\n");
                    }
                    else
                    {
						UINT8* p = (UINT8*) *opt->o_res;
                        KPrintF("--- %s ", p);
                        KPrintF("*******\n");
                    }
                }
            }
        }
        else
        {
            KPrintF("--- NULL ");
            KPrintF("*******\n");
        }
	}
}

//just to print list of opts and info about them
void print_opt_list(pDOSBase DOSBase, struct data* d)
{
	struct MinNode *o_node, *o;
	UINT8 i = 0;
	ForeachNodeSafe(&d->opt_list, o_node, o)
	{
		struct opt_node* opt = (struct opt_node*)o_node;
		KPrintF("opt%d: keyword=%c_%c ", i, *opt->o_start, *opt->o_end);
		KPrintF("opt%d: %s ", i, opt->o_processed?"processed":"not-processed");
		KPrintF("opt%d: %s\n", i, opt->o_has_result?"has result":"no result");
		i++;
	}
}

//just to print list of args and info about them
void print_arg_list(pDOSBase DOSBase, struct data* d)
{
	struct MinNode *a_node, *a;
	UINT8 i = 0;
	ForeachNodeSafe(&d->arg_list, a_node, a)
	{
		struct arg_node* arg = (struct arg_node*)a_node;
		KPrintF("\narg%d: word=%c_%c\n", i, *arg->a_start, *arg->a_end);
		i++;
	}
}


