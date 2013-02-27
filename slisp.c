/*
 * Copyright (C) 2013 XuXinchao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<math.h>
#include<string.h>
#include<stdarg.h>

#define MAXNUM_TOKEN 128
#define MAXNUM_LINE 1000
#define MAXNUM_EXPR 1000
#define OB_MEM 100000
#define BUILTIN_NUM 21
typedef enum {Nil,Boolean,Int,Float,Variable,Symbol,Pair,BuiltinFun} dType;

char * builtin_op []= {"+","-","*","/",">",">=","<","<=","==","%","cons",
                       "car","cdr","and","or","not","is_pair","length",
                       "list","list_ref","append"
                      };
/*
0:syntax error
1:"no more heap space"
2:"unexpected )"
3:contract violation
4:undefined
5:assignment disallowed
6:unknown expression
*/
char * errors[]= {"invalid syntax","no more heap space","unexpected )","contract violation","undefined",
                  "assignment disallowed","unknown expression"
                 };

struct pnode
{
    dType t;
    union
    {
        int nval;
        double dval;
        char * str;
        struct
        {
            struct pnode * car;
            struct pnode * cdr;
        } pcons;
    };
};

struct pnode * globalenvl;
//nil represent in slisp
const struct pnode nil = {.t=Nil,{.nval=0}};
struct pnode * pnil=&nil;
//return when error is happened
const struct pnode err = {.t=Nil,{.nval=14}};
struct pnode * perr=&err;
const struct pnode true_v= {.t=Boolean,{.nval=1}};
struct pnode * pt=&true_v;
const struct pnode false_v= {.t=Boolean,{.nval=0}};
struct pnode * pf=&false_v;


void print_error(int n);
void print_pair(struct pnode *p);
void printpnode();
struct pnode * eval(struct pnode *expr,struct pnode * envl);

/*========symbol string space allocation==========*/
struct slist
{
    struct slist * next;
    char sym[MAXNUM_TOKEN];
};

struct slist * obarray[OB_MEM];

unsigned str_hash(char *s)
{
    unsigned hashval;
    for(hashval=0; *s!='\0'; ++s)
        hashval=31*hashval+*s;
    return hashval%OB_MEM;
}

struct slist * str_lookup(char *s)
{
    int index=str_hash(s);
    struct slist * p;
    for(p=obarray[index]; p!=NULL; p=p->next)
        if(strcmp(p->sym,s)==0)
            return p;
    return NULL;
}

struct slist * str_install(char *s)
{
    struct  slist *p;
    if((p=str_lookup(s))==NULL)
    {
        p=(struct slist *)malloc(sizeof (*p));
        if(p==NULL)
            return NULL;
        int hashval=str_hash(s);
        strcpy(p->sym,s);
        p->next=obarray[hashval];
        obarray[hashval]=p;
        return p;
    }
    return p;
}
/*===================================*/
int is_builtin(char *s)
{
    int i=0;
    for(i=0; i<BUILTIN_NUM; i++)
    {
        if(strcmp(s,builtin_op[i])==0)
            return 1;
    }
    return 0;
}

//====================read=================================
/* get tokens,
 * ( :: 11
 * ) :: 12
 * ' :: 13
 * kong::14
 * int :: Int
 * float :: Float
 * variable: Variable
 */
struct pnode * readtoken()
{
    struct pnode * token;
    char c;
    while(isspace(c=getchar()));/*other whilte space !!!!*/
    if(c=='(')
    {
        token=(struct pnode *)malloc(sizeof (*token));
        token->t=Nil;
        token->nval=11;
        return token;
    }
    else if(c==')')
    {
        token=(struct pnode *)malloc(sizeof (*token));
        token->t=Nil;
        token->nval=12;
        return token;
    }
    else if(c=='\'')
    {
        token=(struct pnode *)malloc(sizeof (*token));
        token->t=Nil;
        token->nval=13;
        return token;
    }
    else if(c=='#')
    {
        c=getchar();
        if(c=='t'||c=='T'||c=='f'||c=='F')
        {
            int t=((c=='t'||c=='T')?1:0);
            c=getchar();
            if(isspace(c)||c=='('||c==')')
            {
                ungetc(c,stdin);
                if(t)
                    return pt;
                else
                    return pf;
            }
            else
            {
                print_error(0);
                token=perr;
                return token;
            }
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else if(c=='+'||c=='-'||c=='*'||c=='/'||c=='%')
    {
        char s[MAXNUM_TOKEN];
        int i=0;
        s[0]=c;
        c=getchar();
        if(isspace(c)||c=='('||c==')')
        {
            s[++i]='\0';
            struct slist * sp=str_install(s);
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            token->t=BuiltinFun;
            token->str=sp->sym;
            ungetc(c,stdin);
            return token;
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else if(c=='>'||c=='<'||c=='=')
    {
        char s[MAXNUM_TOKEN];
        int i=0;
        s[0]=c;
        c=getchar();
        if(isspace(c)||c=='('||c==')')
        {
            s[++i]='\0';
            struct slist * sp=str_install(s);
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            token->t=BuiltinFun;
            token->str=sp->sym;
            ungetc(c,stdin);
            return token;
        }
        else if(c=='=')
        {
            s[++i]=c;
            c=getchar();
            if(isspace(c)||c=='('||c==')')
            {
                s[++i]='\0';
                struct slist * sp=str_install(s);
                struct pnode * token=(struct pnode *)malloc(sizeof(*token));
                if(token==NULL)
                    perror("MEMORY OVERFLOW\n");
                token->t=BuiltinFun;
                token->str=sp->sym;
                ungetc(c,stdin);
                return token;
            }
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else if(isalpha(c)||c=='_')
    {
        char s[MAXNUM_TOKEN];
        int i=0;
        s[0]=c;
        c=getchar();
        while(isalnum(c)||c=='_')
        {
            s[++i]=c;
            c=getchar();
        }
        if(isspace(c)||c=='('||c==')')
        {
            s[++i]='\0';
            struct slist * sp=str_install(s);
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            if(is_builtin(s))
                token->t=BuiltinFun;
            else
                token->t=Variable;
            token->str=sp->sym;
            ungetc(c,stdin);
            return token;
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else if(isdigit(c)&&c!='0')
    {
        char s[MAXNUM_TOKEN];
        int i=0;
        int nn;
        double dn;
        s[0]=c;
        c=getchar();
        int isint=0;
        int isdouble=0;
        while(isdigit(c))
        {
            s[++i]=c;
            c=getchar();
        }
        if(c=='.')
        {
            s[++i]='.';
            c=getchar();
            while(isdigit(c))
            {
                s[++i]=c;
                c=getchar();
            }
            if(isspace(c)||c=='('||c==')')
            {
                s[++i]='\0';
                isdouble=1;
                dn=atof(s);
            }
        }
        else if(isspace(c)||c=='('||c==')')
        {
            s[++i]='\0';
            isint=1;
            nn=atoi(s);
        }
        //ungetc(c,stdin);
        if(isint)
        {
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            token->t=Int;
            token->nval=nn;
            ungetc(c,stdin);
            return token;
        }
        else if(isdouble)
        {
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            token->t=Float;
            token->dval=dn;
            ungetc(c,stdin);
            return token;
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else if(c=='.'||c=='0')
    {
        char s[MAXNUM_TOKEN];
        int isdouble=0;
        double dn;
        int i=0;
        if(c=='0')
        {
            c=getchar();
            if(isspace(c)||c=='('||c==')')
            {
                struct pnode * token=(struct pnode *)malloc(sizeof(*token));
                if(token==NULL)
                    perror("MEMORY OVERFLOW\n");
                token->t=Int;
                token->nval=0;
                ungetc(c,stdin);
                return token;
            }
            else if(c=='.')
                goto dot;
            else
            {
                print_error(0);
                token=perr;
                return token;
            }
        }
dot:
        s[i]='.';
        c=getchar();
        while(isdigit(c))
        {
            s[++i]=c;
            c=getchar();
        }
        if(isspace(c)||c=='('||c==')')
        {
            s[++i]='\0';
            isdouble=1;
            dn=atof(s);
        }
        if(isdouble)
        {
            struct pnode * token=(struct pnode *)malloc(sizeof(*token));
            if(token==NULL)
                perror("MEMORY OVERFLOW\n");
            token->t=Float;
            token->dval=dn;
            ungetc(c,stdin);
            return token;
        }
        else
        {
            print_error(0);
            token=perr;
            return token;
        }
    }
    else
    {
        print_error(0);
        token=perr;
        return token;
    }
}
//========================================================

struct pnode * pconsexpr();
struct pnode * qconsexpr();

/*read the string from input, and store it into cons boxs*/
struct pnode *pconsexpr()
{
    struct pnode * p;
    struct pnode * init;
    struct pnode * parent;
    struct pnode * lch;

    int firsttoken=1;
    lch=readtoken();
    int res;
    if(lch->t==Nil)
    {
        res=lch->nval;
    }
    else
    {
        res=lch->t;
    }
    while (res!=12)
    {
        p=(struct pnode *)malloc(sizeof(*p));
        p->t=Pair;
        if(res==11)
            (p->pcons).car=pconsexpr();
        else if(res==13)
            (p->pcons).car=qconsexpr();
        else
        {
            (p->pcons).car=lch;
        }

        if(firsttoken)
        {
            init=p;
            firsttoken=0;
        }
        else
        {
            (parent->pcons).cdr=p;
        }
        parent=p;
        lch=readtoken();
        if(lch->t==Nil)
        {
            res=lch->nval;
        }
        else
        {
            res=lch->t;
        }
    }

    if(res==12&&firsttoken)
    {
        init=pnil;
        return init;
    }
    else
    {
        (parent->pcons).cdr=pnil;
        return init;
    }
}

struct pnode *qconsexpr()
{
    struct pnode * p;
    struct pnode * lch;
    lch=readtoken();
    int res;
    if(lch->t==Nil)
    {
        res=lch->nval;
    }
    else
    {
        res=lch->t;
    }
    p=(struct pnode *)malloc(sizeof(*p));
    p->t=Symbol;
    (p->pcons).car=pnil;
    if(res==11)
    {
        (p->pcons).cdr=pconsexpr();
    }
    else if(res==12)
    {
        print_error(2);
        return perr;
    }
    else if(res==13)
    {
        (p->pcons).cdr=qconsexpr();
    }
    else
    {
        (p->pcons).cdr=lch;
    }
    return p;
}

/*read from stdin, and parse in s-expression*/
struct pnode *read()
{
    struct pnode * lch;
    struct pnode * ret;
    lch=readtoken();
    int res;
    if(lch->t==Nil)
    {
        res=lch->nval;
    }
    else
    {
        res=lch->t;
    }
    switch(res)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        ret=lch;
        break;
    case 11:
        ret=pconsexpr();
        break;
    case 13:
        ret=qconsexpr();
        break;
    case 12:
        print_error(2);
        ret=perr;
        break;
    case 14:
        ret=perr;
    }
    return ret;
}
//================================================
int is_true(struct pnode *p)
{
    int res;
    if(p->t==Boolean&&p->nval==0)
        res=0;
    else if(p->t==Boolean)
        res=1;
    return res;
}

struct pnode * cons(struct pnode * ca,struct pnode * cd)
{
    if(ca->t==Nil&&cd->t==Nil)
        return pnil;
    else
    {
        struct pnode * p=(struct pnode *) malloc(sizeof(*p));
        p->t=Pair;
        p->pcons.car=ca;
        p->pcons.cdr=cd;
        return p;
    }
}

struct pnode * make_list(int n,...)
{
    va_list ap;
    struct pnode * res;
    struct pnode * p;
    struct pnode * c;

    va_start(ap,n);
    int i=0;
    for(i=0; i<n; i++)
    {
        if(i==0)
        {
            c=cons(va_arg(ap,struct pnode *),pnil);
            res=c;
            p=c;
        }
        else
        {
            c=cons(va_arg(ap,struct pnode *),pnil);
            p->pcons.cdr=c;
            p=c;
        }
    }
    va_end(ap);
    return res;
};

//first
struct pnode * car(struct pnode * p)
{
    if(p==NULL)
    {
        print_error(3);
        return NULL;
    }
    else if(p->t==Pair||p->t==Symbol)
    {
        return p->pcons.car;
    }
    else
    {
        print_error(3);
        return NULL;
    }
}

//except first
struct pnode * cdr(struct pnode * p)
{
    if(p==NULL)
    {
        print_error(3);
        return NULL;
    }
    else if(p->t==Pair||p->t==Symbol)
    {
        return p->pcons.cdr;
    }
    else
    {
        print_error(3);
        return NULL;
    }
}
//second
struct pnode * cadr(struct pnode * p)
{
    return car(cdr(p));
}
//except second
struct pnode * cddr(struct pnode * p)
{
    return cdr(cdr(p));
}
//third
struct pnode * caddr(struct pnode * p)
{
    return car(cddr(p));
}
//except third
struct pnode * cdddr(struct pnode * p)
{
    return cdr(cddr(p));
}
//fourth
struct pnode * cadddr(struct pnode * p)
{
    return car(cdddr(p));
}
//except fourth
struct pnode * cddddr(struct pnode * p)
{
    return cdr(cdddr(p));
}


struct pnode * enclosing_env(struct pnode * envl)
{
    if(envl->t==Pair)
        return cdr(envl);
    return pnil;
}

struct pnode * first_frame(struct pnode * envl)
{
    if(envl->t==Pair)
        return car(envl);
    return pnil;
}

struct pnode * lookup_var (struct pnode *var,struct pnode * envl)
{
    struct pnode * e;
    for(e=envl; e->t!=Nil; e=cdr(e))
    {
        struct pnode * sf=car(e);
        struct pnode * vr;
        struct pnode * vl;
        for(vr=car(sf),vl=cdr(sf); vr->t!=Nil&&vl->t!=Nil; vr=cdr(vr),vl=cdr(vl))
        {
            if(strcmp(car(vr)->str,var->str)==0)
            {
                return car(vl);
            }
        }
    }
    print_error(4);
    return NULL;
}

struct pnode * make_frame(struct pnode * vars,struct pnode *vals)
{
    return cons(vars,vals);
};

struct pnode * frame_vars(struct pnode * f)
{
    return car(f);
};

struct pnode * frame_vals(struct pnode * f)
{
    return cdr(f);
};

void add_binding_to_frame(struct pnode *f,struct pnode *var,struct pnode *val)
{
    if(f->t==Nil)
    {
        f=cons(make_frame(make_list(1,var),make_list(1,val)),pnil);
    }
    else if(f->t==Pair)
    {
        struct pnode * tmp1;
        struct pnode * tmp2;
        tmp1=cons(var,car(f));
        tmp2=cons(val,cdr(f));
        f->pcons.car=tmp1;
        f->pcons.cdr=tmp2;
    }

};

//frame is a pair, car is variable list and cdr is value list
//enviroment is a frame list
struct pnode * extend_env (struct pnode * vars,struct pnode *vals,struct pnode * envl)
{
    struct pnode *f=make_frame(vars,vals);
    struct pnode * p=cons(f,envl);
    return p;
}

void set_variable(struct pnode *var,struct pnode *val,struct pnode * envl)
{
    struct pnode * e;
    for(e=envl; e->t!=Nil; e=cdr(e))
    {
        struct pnode * sf=car(e);
        struct pnode * vr;
        struct pnode * vl;
        for(vr=car(sf),vl=cdr(sf); vr->t!=Nil&&vl->t!=Nil; vr=cdr(vr),vl=cdr(vl))
        {
            if(strcmp(car(vr)->str,var->str)==0)
            {
                vl->pcons.car=val;
            }
        }
    }
    print_error(5);
}

void def_variable (struct pnode *var,struct pnode * val,struct pnode * envl)
{
    int r=-1;
    struct pnode *sf=first_frame(envl);
    struct pnode * vr;
    struct pnode * vl;
    struct pnode * res=eval(val,envl);
    for(vr=car(sf),vl=cdr(sf); vr->t!=Nil&&vl->t!=Nil; vr=cdr(vr),vl=cdr(vl))
    {
        if(strcmp(car(vr)->str,var->str)==0)
        {
            vl->pcons.car=res;
            r=1;
        }
    }
    if(r==-1)
    {
        add_binding_to_frame(sf,var,res);
    }
}

//=================eval===========================
//syntax constructor


int has_tag(struct pnode * expr,char *tag)
{
    char * etag;
    if(expr->t==Pair)
        etag=car(expr)->str;
    else if(expr->t==Variable||expr->t==BuiltinFun)
        etag=expr->str;
    else
        return 0;
    if(strcmp(etag,tag)==0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//(set <var> <value>);
void eval_assignment(struct pnode *expr,struct pnode * envl)
{
    struct pnode * var=car(expr);
    struct pnode * val=eval(cadr(expr),envl);
    set_variable(var,val,envl);
}

//(lambda (<var> <p1> <p2> ...) <body>);
struct pnode * make_lambda(struct pnode * par,struct pnode * body)
{
    struct pnode * lam=(struct pnode *)malloc(sizeof(*lam));
    lam->t=Variable;
    struct slist * s=str_install("lambda");
    lam->str=s->sym;
    return make_list(3,lam,par,body);
}

//(define <var> <value>);
//(define (<var> <p1> <p2> ...)  <body>);
void eval_define(struct pnode *expr,struct pnode * envl)
{
    struct pnode * var=car(expr);
    struct pnode * val;
    if(var->t==Variable)
    {
        val=cadr(expr);
    }
    else
    {
        var=car(car(expr));
        val=make_lambda(cdr(car(expr)),cdr(expr));
    }
    def_variable(var,val,envl);
}

//(if <pre> <con> <alt>);
struct pnode * eval_if(struct pnode *expr,struct pnode * envl)
{
    struct pnode * pred=car(expr);
    struct pnode * conseq=cadr(expr);
    struct pnode * alter=caddr(expr);
    struct pnode * res;
    if(is_true(eval(pred,envl)))
    {
        printpnode(conseq);
        res=eval(conseq,envl);
        printpnode(res);
    }
    else if(alter->t!=Nil)
    {
        printpnode(alter);
        res=eval(alter,envl);
    }
    else
        res=NULL;
    return res;
}
struct pnode * seq_to_expr(struct pnode *p)
{
    if(p->t==Nil)
    {
        return p;
    }
    else if (cdr(p)->t==Nil)
    {
        return car(p);
    }
    else
    {
        struct pnode * b=(struct pnode *)malloc(sizeof(*b));
        b->t=Variable;
        struct slist* s=str_install("begin");
        b->str=s->sym;
        return cons(b,p);
    }
};
struct pnode * trans_cond_to_if(struct pnode *p)
{
    printf("\nclause========\n");
    printpnode(p);

    if(p->t==Nil)
        return pf;
    else
    {
        struct pnode *first=car(p);
        struct pnode *rest=cdr(p);
        struct pnode * pre=car(first);
        struct pnode * e=cdr(first);
        if(pre->t==Variable&&has_tag(pre,"else"))
        {
            if(rest->t==Nil)
            {
                seq_to_expr(e);
            }
            else
            {
                print_error(3);
                return NULL;
            }
        }
        else
        {
            struct pnode * b=(struct pnode *)malloc(sizeof(*b));
            b->t=Variable;
            struct slist* s=str_install("if");
            b->str=s->sym;
            return make_list(4,b,pre,seq_to_expr(e),trans_cond_to_if(rest));
        }
    }
};

struct pnode *make_prodecure(struct pnode *par,struct pnode * body,struct pnode * envl)
{
    struct pnode * p=(struct pnode *)malloc(sizeof(*p));
    p->t=Variable;
    struct slist* s=str_install("procedure");
    p->str=s->sym;
    return make_list(4,p,par,body,envl);
}

struct pnode *eval_seq(struct pnode *expr,struct pnode * envl)
{
    struct pnode *c=expr;
    struct pnode *res;

    while(c->t!=Nil)
    {
        res=eval(car(c),envl);
        c=cdr(c);
    }
    return res;
}

struct pnode *list_of_values(struct pnode *expr,struct pnode * envl)
{
    struct pnode *n=expr;
    struct pnode *c;
    struct pnode *res;
    struct pnode *p;
    int i=0;
    while(n->t!=Nil)
    {
        if(i==0)
        {
            c=cons(eval(car(n),envl),pnil);
            res=c;
            p=c;
        }
        else
        {
            c=cons(eval(car(n),envl),pnil);
            p->pcons.cdr=c;
            p=c;
        }
        n=cdr(n);
        i++;
    }
    return res;
}

//====================built-in function=========================
struct pnode * add(struct pnode * args)
{
    struct pnode * p=(struct pnode *)malloc(sizeof(*p));
    double sumf=0.0;
    int sumn=0;
    int isdouble=0;
    int isch=1;
    struct pnode * c=args;
    while(c->t!=Nil)
    {
        if(car(c)->t==Int&&(!isdouble))
        {
            sumn+=car(c)->nval;
        }
        else if(car(c)->t==Int&&(isdouble))
        {
            if(isch)
            {
                sumf+=sumn;
                isch=0;
            }
            sumf+=car(c)->nval;
        }
        else if(car(c)->t==Float)
        {
            if(isch)
            {
                sumf+=sumn;
                isch=0;
            }
            isdouble=1;
            sumf+=car(c)->dval;
        }
        else
        {
            print_error(3);
            return NULL;
        }
        c=cdr(c);
    }
    if(isdouble)
    {
        p->t=Float;
        p->dval=sumf;
    }
    else
    {
        p->t=Int;
        p->nval=sumn;
    }
    return p;
}

struct pnode * sub(struct pnode * args)
{
    printf("Enter sub\n");
    struct pnode * p=(struct pnode *)malloc(sizeof(*p));
    double sumf=0.0;
    int sumn=0;
    int isdouble=0;
    int isch=1;
    int isfirst=1;
    struct pnode * c=args;
    if((car(c)->t==Int||car(c)->t==Float)&&cdr(c)->t==Nil)
    {
        p->t=car(c)->t;
        if(car(c)->t==Int)
            p->nval=-car(c)->nval;
        else
            p->dval=-car(c)->dval;
        return p;
    }
    while(c->t!=Nil)
    {
        if(car(c)->t==Int&&(!isdouble))
        {
            if(isfirst)
            {
                sumn+=car(c)->nval;
                isfirst=0;
            }
            else
                sumn-=car(c)->nval;

        }
        else if(car(c)->t==Int&&(isdouble))
        {
            if(isch)
            {
                sumf+=sumn;
                isch=0;
            }
            if(isfirst)
            {
                sumf+=car(c)->nval;
                isfirst=0;
            }
            else
                sumf-=car(c)->nval;
        }
        else if(car(c)->t==Float)
        {
            if(isch)
            {
                sumf+=sumn;
                isch=0;
            }
            if(isfirst)
            {
                isdouble=1;
                sumf+=car(c)->dval;
                isfirst=0;
            }
            else
            {
                isdouble=1;
                sumf-=car(c)->dval;
            }
        }
        else
        {
            print_error(3);
            return NULL;
        }
        c=cdr(c);
    }
    if(isdouble)
    {
        p->t=Float;
        p->dval=sumf;
    }
    else
    {
        p->t=Int;
        p->nval=sumn;
    }
    return p;
}

struct pnode * prod(struct pnode * args)
{
    struct pnode * p=(struct pnode *)malloc(sizeof(*p));
    double prof=1;
    int pron=1;
    int isdouble=0;
    int isch=1;
    struct pnode * c=args;
    while(c->t!=Nil)
    {
        if(car(c)->t==Int&&(!isdouble))
        {
            pron*=car(c)->nval;
        }
        else if(car(c)->t==Int&&(isdouble))
        {
            if(isch)
            {
                prof*=pron;
                isch=0;
            }
            prof*=car(c)->nval;
        }
        else if(car(c)->t==Float)
        {
            if(isch)
            {
                prof*=pron;
                isch=0;
            }
            isdouble=1;
            prof*=car(c)->dval;
        }
        else
        {
            print_error(3);
            return NULL;
        }
        c=cdr(c);
    }
    if(isdouble)
    {
        p->t=Float;
        p->dval=prof;
    }
    else
    {
        p->t=Int;
        p->nval=pron;
    }
    return p;
}
struct pnode * division(struct pnode * args)
{
    struct pnode * p=(struct pnode *)malloc(sizeof(*p));
    double divf=1;
    int divn=1;
    int isdouble=0;
    int isch=1;
    int isfirst=1;
    struct pnode * c=args;
    while(c->t!=Nil)
    {
        if(car(c)->t==Int&&(!isdouble))
        {
            if(isfirst)
            {
                divn*=car(c)->nval;
                isfirst=0;
            }
            else
                divn/=car(c)->nval;
        }
        else if(car(c)->t==Int&&(isdouble))
        {
            if(isch)
            {
                divf*=divn;
                isch=0;
            }
            if(isfirst)
            {
                divf*=car(c)->nval;
                isfirst=0;
            }
            else
                divf/=car(c)->nval;
        }
        else if(car(c)->t==Float)
        {
            if(isch)
            {
                divf*=divn;
                isch=0;
            }
            if(isfirst)
            {
                isdouble=1;
                divf*=car(c)->dval;
                isfirst=0;
            }
            else
            {
                isdouble=1;
                divf/=car(c)->dval;
            }
        }
        else
        {
            print_error(3);
            return NULL;
        }
        c=cdr(c);
    }
    if(isdouble)
    {
        p->t=Float;
        p->dval=divf;
    }
    else
    {
        p->t=Int;
        p->nval=divn;
    }
    return p;
}

struct pnode * gt(struct pnode * args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    int r=0;
    double v1;
    double v2;
    if((first->t==Int||first->t==Float)&&(second->t==Int||second->t==Float))
    {
        if(first->t==Int)
            v1=first->nval;
        else
            v1=first->dval;
        if(second->t==Int)
            v2=second->nval;
        else
            v2=second->dval;
        r=(v1>v2);
    }
    if(r==0)
        return pf;
    else
        return pt;
};

struct pnode * ge(struct pnode * args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    int r=0;
    double v1;
    double v2;
    if((first->t==Int||first->t==Float)&&(second->t==Int||second->t==Float))
    {
        if(first->t==Int)
            v1=first->nval;
        else
            v1=first->dval;
        if(second->t==Int)
            v2=second->nval;
        else
            v2=second->dval;
        r=(v1>=v2);
    }
    if(r==0)
        return pf;
    else
        return pt;
};

struct pnode * lt(struct pnode * args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    int r=0;
    double v1;
    double v2;
    if((first->t==Int||first->t==Float)&&(second->t==Int||second->t==Float))
    {
        if(first->t==Int)
            v1=first->nval;
        else
            v1=first->dval;
        if(second->t==Int)
            v2=second->nval;
        else
            v2=second->dval;
        r=(v1<v2);
    }
    if(r==0)
        return pf;
    else
        return pt;
};
struct pnode * le(struct pnode * args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    int r=0;
    double v1;
    double v2;
    if((first->t==Int||first->t==Float)&&(second->t==Int||second->t==Float))
    {
        if(first->t==Int)
            v1=first->nval;
        else
            v1=first->dval;
        if(second->t==Int)
            v2=second->nval;
        else
            v2=second->dval;
        r=(v1<=v2);
    }
    if(r==0)
        return pf;
    else
        return pt;
};

struct pnode * eq(struct pnode * args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    int r=0;
    double v1;
    double v2;
    if((first->t==Int||first->t==Float)&&(second->t==Int||second->t==Float))
    {
        if(first->t==Int)
            v1=first->nval;
        else
            v1=first->dval;
        if(second->t==Int)
            v2=second->nval;
        else
            v2=second->dval;
        r=(v1==v2);
    }
    if(r==0)
        return pf;
    else
        return pt;
};

struct pnode * cons_p(struct pnode *args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    return cons(first,second);
};

struct pnode * car_p(struct pnode *args)
{
    return car(car(args));
};

struct pnode * cdr_p(struct pnode *args)
{
    return cdr(car(args));
};

struct pnode * and_f(struct pnode * args)
{
    int r=1;
    struct pnode * c=args;
    struct pnode * n;
    while(c->t!=Nil)
    {
        n=car(c);
        if(n->t==Boolean&&n->nval==0)
        {
            r=0;
            return pf;
        }
        c=cdr(c);
    }
    return pt;
}

struct pnode * or_f(struct pnode * args)
{
    int r=0;
    struct pnode * c=args;
    struct pnode * n;
    while(c->t!=Nil)
    {
        n=car(c);
        if(n->t==Boolean&&n->nval!=0)
        {
            r=1;
        }
        c=cdr(c);
    }
    if(r==0)
        return pf;
    else
        return pt;
}
struct pnode * not_f(struct pnode * args)
{
    int r=1;
    struct pnode * c=car(args);
    if(c->t==Boolean)
    {
        r=(!c->nval);
    }
    if(r==0)
        return pf;
    else
        return pt;
}

struct pnode * remainder_f(struct pnode *args)
{
    struct pnode *first=car(args);
    struct pnode *second=cadr(args);
    struct pnode * res=(struct pnode *)malloc(sizeof(*res));
    res->t=Int;
    if(first->t==Int&&second->t==Int)
    {
        res->nval=first->nval%second->nval;
        return res;
    }
    else
    {
        print_error(3);
        return NULL;
    }
};

struct pnode * is_pair(struct pnode *args)
{
    if(car(args)->t==Pair)
        return pt;
    else
        return pf;
};

struct pnode * length(struct pnode *args)
{
    struct pnode * res=(struct pnode *)malloc(sizeof(*res));
    struct pnode *c=car(args);
    int i=0;
    while (c->t!=Nil)
    {
        i++;
        c=cdr(c);
    }
    res->t=Int;
    res->nval=i;
    return res;
};
struct pnode * length_value(struct pnode *args)
{
    return length(args)->nval;
};

struct pnode * list_p(struct pnode *args)
{
    return args;
};

struct pnode * list_ref(struct pnode *args)
{
    struct pnode * res=(struct pnode *)malloc(sizeof(*res));
    struct pnode *f=car(args);
    struct pnode *s=cadr(args);
    int i=0;
    int t=0;
    if(s->t==Int)
        t=s->nval;
    else
    {
        print_error(3);//!!!!!Error type
        return NULL;
    }
    if(t>length_value(args))
    {
        print_error(3);//!!!!!!Error type
        return NULL;
    }
    while (f->t!=Nil)
    {
        if(i==t)
            return car(f);
        else
        {
            i++;
            f=cdr(f);
        }
    }
};

struct pnode * append(struct pnode *args)
{
    struct pnode * s1=car(args);
    struct pnode * s2=cadr(args);
    if(s1==Nil)
        return s2;
    else
    {
        struct pnode * c=s1;
        while(cdr(c)->t!=Nil)
        {
            c=cdr(c);
        }
        c->pcons.cdr=s2;
    }
    return s1;
};

//==================================================
struct pnode * apply(struct pnode *pro,struct pnode * args)
{
    if(pro->t==BuiltinFun)
    {
        if(has_tag(pro,builtin_op[0]))
            return add(args);
        else if(has_tag(pro,builtin_op[1]))
            return sub(args);
        else if(has_tag(pro,builtin_op[2]))
            return prod(args);
        else if(has_tag(pro,builtin_op[3]))
            return division(args);
        else if(has_tag(pro,builtin_op[4]))
            return gt(args);
        else if(has_tag(pro,builtin_op[5]))
            return ge(args);
        else if(has_tag(pro,builtin_op[6]))
            return lt(args);
        else if(has_tag(pro,builtin_op[7]))
            return le(args);
        else if(has_tag(pro,builtin_op[8]))
            return eq(args);
        else if(has_tag(pro,builtin_op[9]))
            return remainder_f(args);
        else if(has_tag(pro,builtin_op[10]))
            return cons_p(args);
        else if(has_tag(pro,builtin_op[11]))
        {
            printpnode(args);
            return car_p(args);
        }
        else if(has_tag(pro,builtin_op[12]))
            return cdr_p(args);
        else if(has_tag(pro,builtin_op[13]))
            return and_f(args);
        else if(has_tag(pro,builtin_op[14]))
            return or_f(args);
        else if(has_tag(pro,builtin_op[15]))
            return not_f(args);
        else if(has_tag(pro,builtin_op[16]))
            return is_pair(args);
        else if(has_tag(pro,builtin_op[17]))
            return length(args);
        else if(has_tag(pro,builtin_op[18]))
            return list_p(args);
        else if(has_tag(pro,builtin_op[19]))
            return list_ref(args);
        else if(has_tag(pro,builtin_op[20]))
            return append(args);
    }
    else if(pro->t==Pair)
    {
        struct pnode *t=car(pro);
        if(has_tag(t,"procedure"))
        {
            struct pnode * pars=cadr(pro);
            struct pnode * body=caddr(pro);
            struct pnode * envl=cadddr(pro);
            struct pnode * envl_new=extend_env(pars,args,envl);
            return eval_seq(car(body),envl_new);
        }
    }
    return NULL;
}

/*eval the s-expression*/
struct pnode * eval(struct pnode *expr,struct pnode * envl)
{
    if(expr->t==Nil||expr->t==Boolean||expr->t==Int||expr->t==Float)
        return expr;
    else if(expr->t==Variable)
        return lookup_var(expr,envl);
    else if(expr->t==Symbol)
        return cdr(expr);
    else if(expr->t==BuiltinFun)
        return expr;
    else if(expr->t==Pair)
    {
        if(has_tag(expr,"define"))
        {
            eval_define(cdr(expr),envl);
            return pnil;
        }
        else if(has_tag(expr,"set"))
        {
            eval_assignment(cdr(expr),envl);
            return pnil;
        }
        else if(has_tag(expr,"if"))
            return eval_if(cdr(expr),envl);
        else if(has_tag(expr,"lambda"))
            return make_prodecure(cadr(expr),cddr(expr),envl);
        else if(has_tag(expr,"begin"))
            return eval_seq(cdr(expr),envl);
        else if(has_tag(expr,"cond"))
            return eval(trans_cond_to_if(cdr(expr)),envl);
        else
        {
            struct pnode * pro=eval(car(expr),envl);
            struct pnode * args=list_of_values(cdr(expr),envl);
            printf("pro type is %d\n",pro->t);
            printf("\nargs========\n");
            printpnode(args);
            printf("\n end args========\n");
            if(pro==NULL||args==NULL)
                return NULL;
            return apply(pro,args);
        }
    }
    else
    {
        print_error(6);
        return NULL;
    }
}
//=======================================================================

//==========================print========================================
void print_error(int i)
{
    printf("Error: %s\n",errors[i]);
}

void print_atom(struct pnode *p)
{
    if(p->t==Nil)
        printf("%s","()");
    else if(p->t==Boolean)
    {
        if(p->nval==0)
            printf("%s","#f");
        else if(p->nval==1)
            printf("%s","#t");
    }
    else if(p->t==Int)
        printf("%d",p->nval);
    else if(p->t==Float)
        printf("%f",p->dval);
    else if(p->t==Variable)
        printf("%s",p->str);
    else if(p->t==BuiltinFun)
        printf("<%s builtin function>",p->str);
}

void print_quote(struct pnode * p)
{
    int count=0;
    while(p->t==Symbol)
    {
        printf("(quote ");
        p=(p->pcons).cdr;
        count++;
    }
    if(p->t==Pair)
        print_pair(p);
    else
        print_atom(p);
    while(count!=0)
    {
        printf(")");
        count--;
    }
}

void print_pair(struct pnode *p)
{
    if((p->pcons).cdr->t==Nil)
    {
        printpnode(car(p));
    }
    else
    {
        printf("(");
        printpnode(car(p));
        printf(" ");
        printpnode(cdr(p));
        printf(")");
    }
}

void printpnode(struct pnode *p)
{
    if(p==NULL)
        ;
    else if(p->t==Pair)
    {
        print_pair(p);
    }
    else if(p->t==Symbol)
        print_quote(p);
    else if(p->t==Nil||p->t==Boolean||p->t==Int||p->t==Float||p->t==Variable||p->t==BuiltinFun)
        print_atom(p);
    else
        ;
}
//===========================f====================================

int main()
{
    struct pnode * p;
    struct pnode * res;
    char * in_prompt="=> ";
    char * out_prompt=";value: ";

    struct pnode * addv=(struct pnode *)malloc(sizeof(*addv));
    addv->t=Variable;
    struct slist * ss=str_install("add");
    addv->str=ss->sym;
    struct pnode * fl=make_list(1,addv);
    struct pnode * addl=(struct pnode *)malloc(sizeof(*addv));
    addl->t=Variable;
    addl->str=ss->sym;
    struct pnode * f2=make_list(1,addl);
    globalenvl=pnil;
    globalenvl=extend_env(fl,f2,globalenvl);

    while(1)
    {
        printf("%s",in_prompt);
        p=read();
        //printf("The type of input is %d\n",p->t);
        if(p->t==Nil&&p->nval==14)
        {
            //printf("Input is invalid");
            continue;
        }
        else
        {
            //printf("begin exe\n");
            res=eval(p,globalenvl);
            if(res!=NULL)
            {
                printf("%s",out_prompt);
                printpnode(res);
                printf("\n");
            }
        }
    }
    return 0;
}

