//Matthew Tsenkov
//COSC 341
//Scanner and Parser

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*constants for true and false*/
#define FALSE 0
#define TRUE 1

/*enumerated types for token types*/
typedef enum
{
    ID, INTLITERAL, READ, WRITE, PLUSOP, MINUSOP, ASSIGNOP,
    LPAREN, RPAREN, COMMA, SEMICOLON, SCANEOF,
    LBRACKET, RBRACKET, TIMESOP, DIVIDEOP, MAIN, IF,
    ELSE, WHILE, LESSTHAN, GREATERTHAN, LESSTHANOREQUALTO,
    GREATHERTHANOREQUALTO, EQUALS, NOTEQUALS
} token;

/*function declarations related to scanner*/
token scanner();
void clear_buffer();
void buffer_char(char c);
token check_reserved();
void lexical_error();

/*function declarations related to parser*/
void parser();
void program();
void statement_list();
void statement();
void id_list();
void expression_list();
void expression();
void term();
void add_op();
void match(token tok);
void syntax_error();

void bool_expr();
void operand();
void relational_operator();
void factor();
void mult_op();

/*global variables*/
FILE *fin;              /*source file*/
token next_token;       /*next token in source file*/
char token_buffer[100]; /*token buffer*/
int token_ptr;          /*buffer pointer*/
int line_num = 1;       /*line number in source file*/
int error = FALSE;      /*flag to indicate error*/

/***************************************************************/

/*returns next token from source file*/
token scanner()
{
    char c;

    clear_buffer();

    while(TRUE)
    {
        c = getc(fin);

        if(c == EOF)
            return SCANEOF;

        else if (isspace(c))
        {
            if(c == '\n')
                line_num = line_num + 1;
        }

        else if (isalpha(c))
        {
            buffer_char(c);
            c = getc(fin);
            while (isalnum(c))
            {
                buffer_char(c);
                c = getc(fin);
            }
            ungetc(c, fin);
            return check_reserved();
        }

        /*integer literal*/
        else if (isdigit(c))
        {
            buffer_char(c);
            c = getc(fin);
            while(isdigit(c))
            {
                buffer_char(c);
                c = getc(fin);
            }
            ungetc(c, fin);
            return INTLITERAL;
        }

        else if (c == '(')
            return LPAREN;

        else if (c == ')')
            return RPAREN;

        else if (c == '{')
            return LBRACKET;

        else if (c == '}')
            return RBRACKET;

        else if (c == ',')
            return COMMA;

        else if (c == ';')
            return SEMICOLON;

        else if (c == '+')
            return PLUSOP;

        else if (c == '*')
            return TIMESOP;

        else if (c == '/')
        {
            c = getc(fin);
            if(c == '/')
            {
                do
                    c = getc(fin);
                while(c != '\n');
                line_num = line_num + 1;
            }
            else
            {
                ungetc(c, fin);
                return DIVIDEOP;
            }
        }

        else if (c == '-')
            return MINUSOP;

        else if(c == ':')
        {
            c = getc(fin);
            if(c == '=')
                return ASSIGNOP;
            else
            {
                ungetc(c, fin);
                lexical_error();
            }
        }

        else if (c == '>')
        {
            c = getc(fin);
            if(c == '=')
                return GREATHERTHANOREQUALTO;
            else
            {
                ungetc(c, fin);
                return GREATERTHAN;
            }
        }
        else if (c == '<')
        {
            c = getc(fin);
            if(c == '=')
                return LESSTHANOREQUALTO;
            else
            {
                ungetc(c, fin);
                return LESSTHAN;
            }
        }
        else if (c == '=')
        {
            c = getc(fin);
            if(c == '=')
                return EQUALS;
            else
            {
                ungetc(c, fin);
                lexical_error();
            }
        }

        else if (c == '!')
        {
            c = getc(fin);
            if(c == '=')
                return NOTEQUALS;
            else
            {
                ungetc(c, fin);
                lexical_error();
            }
        }

        else
            lexical_error();
    }
}

/***************************************************************/

/*clears the buffer*/
void clear_buffer()
{
    token_ptr = 0;
    token_buffer[token_ptr] = '\0';
}

/***************************************************************/

/*appends the character to buffer*/
void buffer_char(char c)
{
    token_buffer[token_ptr] = c;
    token_ptr = token_ptr + 1;
    token_buffer[token_ptr] = '\0';
}

/***************************************************************/

/*checks whether buffer is reserved word or identifier*/
token check_reserved()
{
    if(strcmp(token_buffer, "main") == 0)
        return MAIN;
    else if (strcmp(token_buffer, "if") == 0)
        return IF;
    else if (strcmp(token_buffer, "else") == 0)
        return ELSE;
    else if (strcmp(token_buffer, "while") == 0)
        return WHILE;
    else if (strcmp(token_buffer, "read") == 0)
        return READ;
    else if (strcmp(token_buffer, "write") == 0)
        return WRITE;
    else
        return ID;
}

/***************************************************************/

/*reports lexical error and sets error flag*/
void lexical_error()
{
    printf("lexical error in line %d\n", line_num);
    error = TRUE;
}

/***************************************************************/

/*parses source file*/
void parser()
{
    next_token = scanner();
    program();
    match(SCANEOF);
}

/***************************************************************/

/*parses a program*/
/* <program> --> main {<stmtlist>} */
void program()
{
    match(MAIN);
    match(LBRACKET);
    statement_list();
    match(RBRACKET);
}

/***************************************************************/

/*parses list of statements*/
/* <stmtlist> --> <stmt>{<stmt>} */
void statement_list()
{
    statement();
    while(TRUE)
    {
        if(next_token == ID || next_token == READ || next_token == WRITE || next_token == IF ||
                next_token == ELSE || next_token == WHILE)
            statement();
        else
            break;
    }
}

/***************************************************************/



/*parses one statement*/
/*
   <stmt> --> <assignment-stmt>
   <stmt> --> <read-stmt>;
   <stmt> --> <write-stmt>;
   <stmt> --> <if-else> | <if>;
   <stmt> --> <while>;
   */
void statement()
{
    if(next_token == ID)
    {
        match(ID);
        match(ASSIGNOP);
        expression();
        match(SEMICOLON);
    }
    else if (next_token == READ)
    {
        match(READ);
        match(LPAREN);
        id_list();
        match(RPAREN);
        match(SEMICOLON);
    }
    else if (next_token == WRITE)
    {
        match(WRITE);
        match(LPAREN);
        id_list();
        match(RPAREN);
        match(SEMICOLON);
    }
    else if (next_token == WHILE)
    {
        match(WHILE);
        match(LPAREN);
        bool_expr();
        match(RPAREN);
        match(LBRACKET);
        statement_list();
        match(RBRACKET);
    }
    else if (next_token == IF)
    {
        match(IF);
        match(LPAREN);
        bool_expr();
        match(RPAREN);
        match(LBRACKET);
        statement_list();
        match(RBRACKET);
        if(next_token == ELSE)
        {
            match(LBRACKET);
            statement_list();
            match(RBRACKET);
        }
    }
    else
        syntax_error();
}

/* <bool-expr> -> <operand> <relational-operator> <operand> */
void bool_expr()
{
    operand();
    relational_operator();
    operand();
}

/* <operand> --> ID | INTLITERAL */
void operand()
{
    if(next_token == ID || next_token == INTLITERAL)
        match(next_token);
    else
        syntax_error();
}

/*<relational-operator> --> < | > | <= | >= | == | != */
void relational_operator()
{
    if(next_token == GREATERTHAN || next_token == LESSTHAN || next_token == GREATHERTHANOREQUALTO ||
            next_token == LESSTHANOREQUALTO || next_token == EQUALS || next_token == NOTEQUALS)
        match(next_token);
    else
        syntax_error();
}

/***************************************************************/

/*parses list of identifiers*/
/* <idlist> --> id{,id} */
void id_list()
{
    match(ID);
    while(next_token == COMMA)
    {
        match(COMMA);
        match(ID);
    }
}

/***************************************************************/

/*parses list of expressions*/
/* <explist> --> <exp>{,<exp>} */
void expression_list()
{
    expression();
    while(next_token == COMMA)
    {
        match(COMMA);
        expression();
    }
}

/***************************************************************/

/*parses one expression*/
/* <exp> --> <term>{<adop><term>}*/
void expression()
{
    term();
    while(next_token == PLUSOP || next_token == MINUSOP)
    {
        add_op();
        term();
    }
}
/***************************************************************/

/*parses one term*/
/* <term> --> <term> * <factor> | <term> / <factor> | <factor>*/
void term()
{
    factor();
    while(next_token == TIMESOP || next_token == DIVIDEOP)
    {
        mult_op();
        factor();
    }
}

/*parses one term*/
/*
   <factor> --> id
   <factor> --> integer
   <factor> --> (<expr>)
   */
void factor()
{
    if(next_token == ID)
        match(ID);
    else if(next_token == INTLITERAL)
        match(INTLITERAL);
    else if(next_token == LPAREN)
    {
        match(LPAREN);
        expression();
        match(RPAREN);
    }
    else
        syntax_error();
}

/***************************************************************/

/*parses times or divides operator*/
/* <multop> --> *|/ */
void mult_op()
{
    if(next_token == TIMESOP || next_token == DIVIDEOP)
        match(next_token);
    else
        syntax_error();
}

/***************************************************************/

/*parses plus or minus operator*/
/* <adop> --> +|- */
void add_op()
{
    if(next_token == PLUSOP || next_token == MINUSOP)
        match(next_token);
    else
        syntax_error();
}

/***************************************************************/

/*checks whether the expected token and the actual token match,
and also reads the next token from source file */
void match(token tok)
{
    if(tok == next_token)
        ;
    else
        syntax_error();

    next_token = scanner();
}

/***************************************************************/

/*reports syntax error*/
void syntax_error()
{
    printf("syntax error in line %d\n", line_num);
    error = TRUE;
}

/***************************************************************/

int main()
{
    int user_input;

    printf("Enter an Option.\n1. To scan a source code file and write sequence of tokens.\n2. To parse a source code file.\n");
    scanf("%d", &user_input);

    char inF[100];

    if(user_input == 1) //If user input is 1
    {
        printf("Enter an input file, might need a .txt extension: ");
        scanf("%s", inF); //add user input to inF array
        fin = fopen(inF, "r"); //read file and open it into fin
        if(fin == NULL)
            printf("\nFile not found.");
        else
        {
            int pos_token_scanner; //The position of the tokens in the scanner

            FILE *fout;
            char outF[100]; //Array that will store the name of the output file

            printf("Enter an output file: ");
            scanf("%s", outF);

            fout = fopen(outF, "w"); //make the write file to which we output

            /*While the token is not at position 11, which is
                SCANEOF or scan end of file we want to be looping.*/
            while(pos_token_scanner != 11)
            {
                pos_token_scanner = scanner();

                if(pos_token_scanner == 0)
                    fprintf(fout, "ID ");
                else if (pos_token_scanner == 1)
                    fprintf(fout, "INTLITERAL ");
                else if (pos_token_scanner == 2)
                    fprintf(fout, "READ ");
                else if (pos_token_scanner == 3)
                    fprintf(fout, "WRITE ");
                else if (pos_token_scanner == 4)
                    fprintf(fout, "PLUSOP ");
                else if (pos_token_scanner == 5)
                    fprintf(fout, "MINUSOP ");
                else if (pos_token_scanner == 6)
                    fprintf(fout, "ASSIGNOP ");
                else if (pos_token_scanner == 7)
                    fprintf(fout, "LPAREN ");
                else if (pos_token_scanner == 8)
                    fprintf(fout, "RPAREN ");
                else if (pos_token_scanner == 9)
                    fprintf(fout, "COMMA ");
                else if (pos_token_scanner == 10)
                    fprintf(fout, "SEMICOLON\n");
                else if (pos_token_scanner == 11)
                    fprintf(fout, "SCANEOF");
                else if (pos_token_scanner == 12)
                    fprintf(fout, "\nLBRACKET\n");
                else if (pos_token_scanner == 13)
                    fprintf(fout, "RBRACKET\n");
                else if (pos_token_scanner == 14)
                    fprintf(fout, "TIMESOP ");
                else if (pos_token_scanner == 15)
                    fprintf(fout, "DIVIDEOP ");
                else if (pos_token_scanner == 16)
                    fprintf(fout, "MAIN ");
                else if (pos_token_scanner == 17)
                    fprintf(fout, "IF ");
                else if (pos_token_scanner == 18)
                    fprintf(fout, "ELSE ");
                else if (pos_token_scanner == 19)
                    fprintf(fout, "WHILE ");
                else if (pos_token_scanner == 20)
                    fprintf(fout, "LESSTHAN ");
                else if (pos_token_scanner == 21)
                    fprintf(fout, "GREATERTHAN ");
                else if (pos_token_scanner == 22)
                    fprintf(fout, "LESSTHANOREQUALTO ");
                else if (pos_token_scanner == 23)
                    fprintf(fout, "GREATHERTAHNOREQUALTO ");
                else if (pos_token_scanner == 24)
                    fprintf(fout, "EQUALS ");
                else if (pos_token_scanner == 25)
                    fprintf(fout, "NOTEQUALS ");
            }
        }
    }
    else if (user_input == 2) //If user input is 2
    {
        printf("Enter an input file, might need a .txt extension: ");
        scanf("%s", inF);
        fin = fopen(inF, "r");
        if(fin == NULL)
            printf("\nFile not found.");
        else
        {
            parser();

            if(error != TRUE)
                printf("Parsing successful");
        }
    }
    else
        printf("Invalid Input.");
    return 0;
}


/***************************************************************/
