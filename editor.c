#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 10000// максимальная длина строки
#define StrOutFirstDefault 0 //начальная позиция строки по умолчанию
#define StrOutLastDefault 40//конечная позиция строки по умолчанию
#define StrStyleDefaut 4//выравнивание по умолчанию (1 - по левому краю, 2 - по правому, 3 - по центру, 4 - по ширине)
#define SwitchSmartFbBoards 1 //переключение режима переноса слов с другим размером строки при выравнивании по ширине(0 - не переносить, 1 - не переносить)
#define ParagraphSpaceStr 1//переключение режима разделения абзацев(0 - нет отступа; 1 - есть отступ)

#define TRUE 1
#define FALSE 0

int StrOutFirst = StrOutFirstDefault;// текущее начальное положение строки
int StrOutLast = StrOutLastDefault;// текущее конечное\ положение строки
int StrStyle = StrStyleDefaut;// текущий стиль строки
int LineCtr = 0;// счётчик слов (глобальный для удобности использования)
int FbCounter = 0;// счётчик строк, выравненных по ширине 
int FbFlg = FALSE;// флаг для понимания границ выравнивания по ширине
int SwitchSmartFbBoardsFlg = FALSE;// флаг для установки нужного режима границ выравнивания по ширине
int ParagraphSpaceStrFlg = FALSE;// флаг для утсановки нужного режима разделения абзацев 

char** Buff;//двумерный динамический массив для записи строк для форматирования по ширине
int** BuffBoards;//двумерный динамический массив для записи границ строк для форматирования по ширине

FILE *file_in, *file_out; // объявление указателей на файлы

void CheckLineBorders(char *str);// функция поиска команд форматирования конечного и начального положения строки
void CheckLineStyle(const char *str);// функция поиска команд форматирования стиля строки
void PutLineInFile(char *str, int curlinectr);// функция вывода данных
void PutFbInFile();//вспомогательная функция вывода данных для выравнивания по ширине

int main(){

    char line[MAXLEN] = { };// объявление массива, в которы будет помещена строка
    char *lineptr;// объявление указателя на данную строку

    file_in = fopen("bl_in.txt", "rt");// открытие файла для чтения
    if(file_in == NULL)
        return 1;

    file_out = fopen("bl_out.txt", "wt");// открытие файла для записи
    if(file_out == NULL)
        return 1;

    while(!feof(file_in)){//цикл жля считования количества строк
        lineptr = fgets(line, MAXLEN, file_in);
        if (lineptr == NULL)
            break;

        LineCtr++;
    }

    file_in = fopen("bl_in.txt", "rt");//повторное открытие файла для чтения
    if(file_in == NULL)
        return 1;

    Buff = (char**)calloc(LineCtr, sizeof(char*));
    BuffBoards = (int**)calloc(LineCtr, sizeof(int*));

    int curlinectr = 1;//текущая строка
    while(!feof(file_in)){ //цикл до конца файла
        lineptr = fgets(line, MAXLEN, file_in);// чтение строки
        if (lineptr == NULL) // файл исчерпан
            break;

        ParagraphSpaceStrFlg = FALSE;// состояние флага по умолчанию
        SwitchSmartFbBoardsFlg = FALSE;// состояние флага по умолчанию
        CheckLineBorders(line);//поиск команд форматирования конечного и начального положения строки
        CheckLineStyle(line);// поиск команд форматирования стиля строки
        PutLineInFile(line, curlinectr);// вывод данных в файл
        curlinectr++;
    }

    fclose(file_in);
    fclose(file_out);

    free(Buff);//удаление массивов указателей
    free(BuffBoards);
    return 0;
}

void CheckLineBorders(char *str){// функция поиска команд форматирования конечного и начального положения строки
char *lineptr = str;// указатель на полученную строку

while(*lineptr != '\0'){ // до конца строки 

    if(*lineptr == '%' && *(lineptr + 1) == 'b'&& *(lineptr+2) >= '0' && *(lineptr+2) <= '9'){ // поиск комманд форматирования начальной позиции строки

        lineptr += 2;
        int i = 0;
        char strbord[1000] = {};

        while(isdigit(*lineptr)){ 
            strbord[i++] = *lineptr++;
        }

        StrOutFirst = atoi(strbord);

        #if SwitchSmartFbBoards == 0 
            SwitchSmartFbBoardsFlg = TRUE;
        #endif

        #if ParagraphSpaceStr == 1
            ParagraphSpaceStrFlg = TRUE;
        #endif
    }

    if(*lineptr == '%' && *(lineptr + 1) == 'e'&& *(lineptr+2) >= '0' && *(lineptr+2) <= '9'){ // поиск комманд форматирования конечной позиции строки

        lineptr += 2;
        int i = 0;
        char strbord[1000] = {};

        while(isdigit(*lineptr)){
            strbord[i++] = *lineptr++;
        }
        
        StrOutLast = atoi(strbord);

        #if SwitchSmartFbBoards == 0
            SwitchSmartFbBoardsFlg = TRUE;
        #endif

        #if ParagraphSpaceStr == 1
            ParagraphSpaceStrFlg = TRUE;
        #endif
    }

    lineptr++;
}
}

void CheckLineStyle(const char* str){
    const char* lineptr = str;
    while(*lineptr){
        if(*lineptr == '%' && *(lineptr + 1) == 'f'){
            switch (*(lineptr+2)){
                case 'l':
                case 'r':
                case 'c':
                case 'b':
                    #if ParagraphSpaceStr == 1
                        ParagraphSpaceStrFlg = TRUE;
                    #endif
                    StrStyle = *(lineptr+2) - '0';
                    break;
            }
            lineptr += 3;
        }
        else{
            lineptr++;
        }
    }
}



void PutLineInFile(char *str, int curlinectr){// функция вывода данных

    if(StrOutLast - StrOutFirst < 15) // проверка адекватности условия
        return;

    char *lineptr = str;//указатель на текущую строку
    char stredit[MAXLEN] = {}; // массив символов для отредактиованной строки
    int i = 0;//счётчик
    int charctr = 0;//счётчик символов

    int flg = FALSE;
    int flg1 = FALSE;
    int flg2  = FALSE;
    while (*lineptr != '\0'){//удаление из строк меток
        flg = FALSE;
        flg1 = FALSE;
        flg2  = FALSE;

        if(*lineptr == '%' && *(lineptr + 1) == 'f' && (*(lineptr + 2) == 'd' || *(lineptr + 2) == 'l'|| *(lineptr + 2) == 'r'
        || *(lineptr + 2) == 'c'|| *(lineptr + 2) == 'b')){//поиск меток стился строки
            flg2 = TRUE;
            if(*(lineptr + 3) == ' ' && *(lineptr - 1) == ' '){
                lineptr +=4;
                flg = TRUE;
            } else{
                lineptr +=3;
            }
        }
        if(*lineptr == '%' && (*(lineptr+1) == 'b' || *(lineptr+1) == 'e') && *(lineptr+2) >= '0'&& *(lineptr+2) <= '9'){//поиск меток размера строки
            flg2 = TRUE;
            if(*(lineptr-1) == ' '){
                flg1 = TRUE;
            }
            lineptr+=2;
            while(*lineptr >='0' && *lineptr <= '9'){
                lineptr++;
            }
            if(*lineptr == ' ' && flg1){
                lineptr++;
                flg = TRUE;
            }
        }
        if(*lineptr == '%' && flg2){
            flg = TRUE;
        }
        if(flg == FALSE){
            stredit[i++] = *lineptr++;
            charctr++;
        }
    }

    char curline[MAXLEN] = {};//массив для выывода строки
    

    int spaceflg = FALSE;//флаг пустой строки
    char *countptr = stredit; // указатель на отредактированную строку для подсчёта числа слов/букв/пробелов
    lineptr = stredit; // указатель на отредактированную строку
    int j = 0;

    if(stredit[0] == '\n'){//проверка на пустую строку
        spaceflg = TRUE;
    } 
    if(curlinectr == 1){// отсутствие отсуп перед первой строкой
        ParagraphSpaceStrFlg = FALSE;
    }
    
    if(stredit[0] == ' '){//проверка на абзац и на пустую строку с пробелами
        #if ParagraphSpaceStr == 1
            if(curlinectr != 1){ParagraphSpaceStrFlg = TRUE;}
        #endif
        while(stredit[j] ==' '){
            j++;
        }
        if(stredit[j] == '\n'){
            spaceflg = TRUE;
        }
    }

    if((StrStyle != 4 || stredit[0] == ' ' || spaceflg || SwitchSmartFbBoardsFlg) && FbFlg){//вывод строк, отформатированных по ширине
        PutFbInFile();//вызов функции вывода для строк, выравненных по ширине
        FbFlg = FALSE;
        for(int i2 = 0; i2 < FbCounter; i2++){//удаление массивов строк
            free(Buff[i2]);
        }
        for(int i2 = 0; i2 < FbCounter; i2++){//удаления массива границ
            free(BuffBoards[i2]);
        }
        FbCounter = 0;//обнуление счётчика строк для форматирования по ширине
    }
        
    if(StrStyle == 4 && !FbFlg && !spaceflg && ParagraphSpaceStrFlg){//вывод пустой строки при разделении абзацев для выравнивания по ширине
        fputc('\n', file_out);
    }

    if(StrStyle == 4 && !FbFlg && spaceflg && !ParagraphSpaceStrFlg){//вывод пустой строки для выравнивания по ширине
        fputc('\n', file_out);
    }


    switch (StrStyle) // поиск нужного стиля вывод и вывод строки
    {
    case 1: //форматирование по левому краю
        if(ParagraphSpaceStrFlg && !spaceflg){//вывод пустой строки при разделении абзацев
            fputc('\n', file_out);
        }
        if(spaceflg && !ParagraphSpaceStrFlg){//вывод пустой строки
            fputc('\n', file_out);
            break;
        }      
        while(*lineptr != '\0'){//чикл до конца строки
                memset(curline, 0, MAXLEN);//очищение массива для текущей строки
                i = 0;//счётчик
                do{//цикл считывания строки до конца доступной области вывода
                    while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                    *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){//считывание строки пословно
                        curline[i] = *lineptr;
                        lineptr++;
                        i++;
                    }
                    if(*lineptr == '\0'){//проверка на конец строки
                        break;
                    }
                    if(i >= (StrOutLast - StrOutFirst) && curline[0]!='\0'){//перенос обрезанного слова на следующую строку
                        lineptr--;
                        i--;
                        while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                        *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){
                            lineptr--;
                            curline[i] = '\0';
                            i--;
                        }
                        lineptr++;
                        i++;
                        break;
                    }
                    if(*lineptr != '\n'){//избегание считывания лишних знаков новой строки
                        curline[i] = *lineptr;
                    }
                    lineptr++;
                    i++;
                if(i >= (StrOutLast - StrOutFirst) && *lineptr ==' '){
                    lineptr++;
                    break;
                }
                } while( i < (StrOutLast - StrOutFirst));
                if(curline[0]!='\0'){//вывод строки в файл
                    for(int i2 = 0; i2 < StrOutFirst; i2++){
                        fputc(' ', file_out);
                    }
                    fputs(curline, file_out);

                    if(curlinectr != LineCtr){
                        fputc('\n', file_out);
                    } else if(*lineptr !='\0'){
                        fputc('\n', file_out);
                    }
                }
            }
        break;
    case 2://форматирование по правому краю
        if(ParagraphSpaceStrFlg && !spaceflg){//вывод пустой строки при разделении абзацев
            fputc('\n', file_out);
        }
        if(spaceflg && !ParagraphSpaceStrFlg){//вывод пустой строки
            fputc('\n', file_out);
            break;
        }
        while(*lineptr != '\0'){//цикл до конца строки
                    memset(curline, 0, MAXLEN);//очищение массива для текущей строки
                    i = 0;//счётчик
                    do{//цикл считывания строки до конца доступной области вывода
                        while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                        *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){//считывание строки пословно
                            curline[i] = *lineptr; 
                            lineptr++;
                            i++;
                        }
                        if(*lineptr == '\0'){
                            break;
                        }
                        if(i >= (StrOutLast - StrOutFirst)){//перенос обрезанного слова на следующую строку
                            lineptr--;
                            i--;
                            while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                            *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){
                                curline[i] = '\0';
                                lineptr--;
                                i--;
                            }
                            lineptr++;
                            i++;
                            break;
                        }
                        if(*lineptr != '\n'){//избегание считывания лишних знаков новой строки
                            curline[i] = *lineptr;
                        }
                        lineptr++;
                        i++;
                    } while( i < (StrOutLast - StrOutFirst));
                    while(curline[i] == ' ' || curline[i] == '\n' || curline[i] == '\0'){//удаление лишних символов в конце строки
                        curline[i] = '\0';
                        i--;
                    }
                    i++;
                    int h = (StrOutLast - StrOutFirst - i);//расчёт числа нужных пробелов длы выравнивания по правому краю

                    if(curline[0]!='\0'){//вывод строки в файл
                    for(int i1 = 0; i1 < StrOutFirst; i1++){
                        fputc(' ', file_out);
                    }
                    for(int i1 = 0; i1 < h; i1++){
                        fputc(' ', file_out);
                    }
                    fputs(curline, file_out);

                    if(curlinectr != LineCtr){
                        fputc('\n', file_out);
                    } else if(*lineptr !='\0'){
                        fputc('\n', file_out);
                    }

                    }
                }
        break;
    case 3://форматирование по центру
        if(ParagraphSpaceStrFlg && !spaceflg){//вывод пустой строки при разделении абзацев
            fputc('\n', file_out);
        }
        if(spaceflg && !ParagraphSpaceStrFlg){//вывод пустой строки
            fputc('\n', file_out);
            break;
        }
        while(*lineptr != '\0'){//чикл до конца строки
                    memset(curline, 0, MAXLEN);//очищение массива для текущей строки
                    i = 0;//счётчик
                    do{//цикл считывания строки до конца доступной области вывода
                        while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                        *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){//считывание строки пословно
                            curline[i] = *lineptr; 
                            lineptr++;
                            i++;
                        }
                        if(*lineptr == '\0'){//проверка на конец строки
                            break;
                        }
                        if(i >= (StrOutLast - StrOutFirst)){//перенос обрезанного слова на следующую строку
                            lineptr--;
                            i--;
                            while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                            *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){
                                lineptr--;
                                curline[i] = '\0';
                                i--;
                            }
                            lineptr++;
                            i++;
                            break;
                        }
                        if(*lineptr != '\n'){//избегание считывания лишних знаков новой строки
                            curline[i] = *lineptr;
                        }
                        lineptr++;
                        i++;
                    } while( i < (StrOutLast - StrOutFirst));
                    while(curline[i] == ' ' || curline[i] == '\n' || curline[i] == '\0'){//удаление лишних символов в конце строки
                        curline[i] = '\0';
                        i--;
                    }
                    int h = (StrOutLast - StrOutFirst - i)/2;//расчёт числа нужных пробелов длы выравнивания по ширине
                    if(curline[0]!='\0'){//вывод строки в файл
                    for(int i1 = 0; i1 < StrOutFirst; i1++){
                        fputc(' ', file_out);
                    }
                    for(int i1 = 0; i1 < h; i1++){
                        fputc(' ', file_out);
                    }
                    fputs(curline, file_out);
                    if(curlinectr != LineCtr){
                        fputc('\n', file_out);
                    } else if(*lineptr !='\0'){
                        fputc('\n', file_out);
                    }
                    }
                }
        break;
    case 4://форматирование по ширине
        if(spaceflg){//избегание считывания пустой строки
            break;
        }
        Buff[FbCounter] = (char*)calloc(MAXLEN, sizeof(char));//выделение памяти под новую строку

        for(int i2 = 0; i2 < MAXLEN; i2++){//перенос строки в двумерный динамический массив
            Buff[FbCounter][i2] = stredit[i2];
            if(stredit[i2] == '\0'){
                break;
            }
        }

        BuffBoards[FbCounter] = (int*)calloc(2, sizeof(int));//выделение памяти под новые границы строки
        BuffBoards[FbCounter][0] = StrOutFirst;//перенос текущих границ в двумерный динамический массив 
        BuffBoards[FbCounter][1] = StrOutLast;

        FbCounter++;
        FbFlg = TRUE;//флаг считывания строк для выравнивания по ширине
       break;
    default:
     break;
    }

    if(curlinectr == LineCtr && FbFlg){// вспомогательный вывод строк, отформатированных по ширине
        PutFbInFile();//вызов функции вывода для строк, выравненных по ширине
        FbFlg = FALSE;
        for(int i2 = 0; i2 < FbCounter; i2++){//удаление массивов строк
            free(Buff[i2]);
        }
        for(int i2 = 0; i2 < FbCounter; i2++){//удаления массива границ
            free(BuffBoards[i2]);
        }
        FbCounter = 0;//обнуление счётчика строк для форматирования по щирине
    }
} 
void PutFbInFile(){// вспомогательная функция вывода данных для выравнивания по ширине
    char *Bufffb = (char*)calloc(MAXLEN*FbCounter, sizeof(char));// выделение памяти для одномерного динамического массива строк
    int bufffbctr = 0;// счётчик строк
    
    for(int i = 0; i < FbCounter; i++){// перенос строк из двумерного массива в одномерный для удобноц работы с ними
        for(int j = 0; j < MAXLEN; j++){
            if(Buff[i][j] == '\0'){
                break;
            }
            Bufffb[bufffbctr] = Buff[i][j];
            bufffbctr++;    
        }
    }
    char curline[MAXLEN];// массив для текущей строки
    char *lineptr = Bufffb;// указатель на массив строк
    int strctr = 0;//счётчик строк
    int strsave = 0;// количесво строк
    int strbrdctr = 0;// счётчик для границ строки
    int i = 0;// счётчик
    int flg = FALSE;// флаг для верного расчёта количества строк
    int *boardinptr = BuffBoards[i];
    int *boardoutptr = BuffBoards[i];

    while(*lineptr != '\0'){// цикл до конца массива для расчёта количества строк, дублирющий цикл для вывода
        boardinptr = &BuffBoards[strbrdctr][0];// указатели на нужные границы
        boardoutptr = &BuffBoards[strbrdctr][1];
        i = 0;
        flg = FALSE;
        if(strctr == 0){// красная строка
                while(*lineptr ==' '){
                    lineptr++;
                    i++;
                }
            }
        do{//цикл до конца доступной области вывода
            while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
            *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){//считывание строки пословно
                lineptr++;
                i++;
            }
            if(*lineptr == '\0'){//проверка на конец строки
                flg = TRUE;
                strctr++;//счётчик строк
                break;
            }
            if(i >= (*boardoutptr - *boardinptr)){//перенос обрезанного слова на следующую строку
                lineptr--;
                i--;
                while( *lineptr != ' ' && *lineptr != '.' && *lineptr!= ',' && *lineptr!= ';' && *lineptr != ')' && 
                *lineptr!= ':' && *lineptr!= '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){
                    lineptr--;
                    i--;
                }
                lineptr++;
                strctr++;//счётчик строк
                flg = TRUE;
                i++;
                break;
            }

            if(*lineptr == '\n' && strbrdctr < (FbCounter-1)){// счётчик границ
                strbrdctr++;
            }

            if(*lineptr != '\n'){// считывание символов разделителей
                i++;
            } else if(*(lineptr-1) != ' '){
                i++;
            } else{
                while(*lineptr == ' '){
                    i--;
                    lineptr--;
                }
            }
            lineptr++;
            if(i >= (*boardoutptr - *boardinptr) && *lineptr == ' '){
                lineptr++;
                break;
            }
        }while(i < (*boardoutptr - *boardinptr));
        if(flg == FALSE){//счётчик строк
        strctr++;
        }
    }
    strsave = strctr;
    lineptr = Bufffb;

    strbrdctr = 0;
    strctr = 0; 

    while(*lineptr != '\0'){//цикл вывода до конца массива
        boardinptr = &BuffBoards[strbrdctr][0];// указатели на нужные границы
        boardoutptr = &BuffBoards[strbrdctr][1];
        
        memset(curline, 0, MAXLEN);
        int inwordflg = FALSE;
        flg = FALSE;
        i = 0;
        int wordctr = 0;// счётчик слов
        int spaceflg = FALSE;
        for(int i1 = 0; i1 < *boardinptr; i1++){
            fputc(' ', file_out);
        }
        i = 0;
         if(strctr == 0){//считывание красной строки
                  while(*lineptr == ' '){
                    curline[i] = *lineptr;
                    lineptr++;
                    i++;
                }
            }
        do{//цикл считывания строки до конца доступной области вывода
            while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
            *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){//считывание строки пословно
                curline[i] = *lineptr;
                lineptr++;
                i++;
                inwordflg = TRUE;
            }
            if(*lineptr == '\0'){//проверка на конец строки
                flg = TRUE;
                strctr++;//счётчик строк
                break;
            }
            if(i >= (*boardoutptr - *boardinptr)){//перенос обрезанного слова на следующую строку
                lineptr--;
                i--;
                while( *lineptr != ' ' && *lineptr != '.' && *lineptr != ',' && *lineptr != ';' && *lineptr != ')' && 
                *lineptr != ':' && *lineptr != '(' && *lineptr != '!' && *lineptr != '?'&& *lineptr != ' '&& *lineptr != '\n'&& *lineptr != '\0'){
                    lineptr--;
                    curline[i] = '\0';
                    i--;
                }
                lineptr++;
                strctr++;//счётчик строк
                flg = TRUE;
                i++;
                break;
            }
            if((*lineptr == ' ' || *lineptr == '\n')&& inwordflg){// счётчик слов
                wordctr++;
                inwordflg = FALSE;
            }
            if(*lineptr == '\n' && strbrdctr < (FbCounter-1)){// счётчик границ
                strbrdctr++;
            }
            if(*lineptr != '\n'){// считывание символов разделителей
                curline[i] = *lineptr;
                i++;
            } else if(*(lineptr-1) != ' '){
                curline[i] = ' ';
                i++;
            } else{
                while(*lineptr == ' '){
                    curline[i] = '\0';
                    i--;
                    lineptr--;
                }
                curline[i] = ' ';
            }
            lineptr++;
            if(i >= (*boardoutptr - *boardinptr) && *lineptr == ' '){
                lineptr++;
                break;
            }
        } while( i < (*boardoutptr - *boardinptr));
        if(flg == FALSE){//счётчик строк
            strctr++;
        }
        if(strctr == strsave){// вывод последней строки по левому краю
            fputs(curline, file_out);
                fputc('\n', file_out);
            break;
        }
        while(curline[i] == ' ' || curline[i] == '\n' || curline[i] == '\0'){
            curline[i] = '\0';
            i--;
        }
        i++;
        int h = (*boardoutptr - *boardinptr) - i;// рассчёт нужного числа пробелов
        int h1 = h % (wordctr-1);
        int j = 0;// счётчик для вывода

        if(strctr == 1){// вывод красной строки
            while(curline[j] == ' '){
                fputc(curline[j], file_out);
                j++;
            }
        }
        if(strctr != 1 && curline[0] == ' '){// защита от пробелов в начале строки
            j++;
            h1++;
        }
        while(curline[j]!= '\0'){// вывод строки посимвольно с нужным количеством пробелов
            if(curline[j] == ' ' ){
                for(int i1 = 0; i1 < h/(wordctr-1); i1++){
                    fputc(' ', file_out);
                }
                if(h1 > 0){
                    fputc(' ', file_out);
                    h1--;
                }
            }
            fputc(curline[j], file_out);
            
            j++;
        }
            fputc('\n', file_out);
        
    }
    free(Bufffb);// удаление одномеронго массива строк
   
}