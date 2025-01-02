#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include "curses.h"  
    enum SpecialKey {
        Enter = 10,
        Back_Space = 8,
        U_Arrow = 259,
        D_Arrow= 258,
        L_Arrow = 260,
        R_Arrow = 261,
        CtrlQ = 17,
        CtrlS = 19,
        CtrlF = 6,
        Numpad_Home = 449,
        Numpad_End = 455 ,
        Numpad_PgUp = 451,
        Numpad_PgDn = 457,
        Esc =27 
    };

#elif __linux__
    #include <ncurses.h>   
    enum SpecialKey {
        Enter = 10,
        Back_Space = 263,
        U_Arrow = 259,
        D_Arrow = 258,
        L_Arrow = 260,
        R_Arrow = 261,
        CtrlQ = 17,
        CtrlS = 19,
        CtrlF = 6,
        Numpad_Home = 262,
        Numpad_End = 360,
        Numpad_PgUp = 339,
        Numpad_PgDn = 338,
        Esc = 27
    };
#elif __APPLE__
    #include <ncurses.h>
    enum SpecialKey {
        Enter = 10,
        Back_Space = 127,
        U_Arrow = 259,
        D_Arrow = 258,
        L_Arrow = 260,
        R_Arrow = 261,
        CtrlQ = 17,
        CtrlS = 19,
        CtrlF = 6,
        Numpad_Home = 449,
        Numpad_End = 455,
        Numpad_PgUp = 451,
        Numpad_PgDn = 457,
        Esc = 27
        };
#else
#endif

//page up, down과 스크롤, 기타 조금 문제 있는 것들 다듬고 맥과 리눅스 키 알기, 저장, 찾기, 

struct line_{//단락을 나타내는 자료 구조
    struct line_ * ptr1;//위쪽 포인터
    struct line_  * ptr2;//아래쪽 포인터
    unsigned int num;//배열의 입력수
    size_t size;//문자열 크기
    char * text;//문장 
};
typedef struct line_ line;

line * header = NULL; //헤더
line * present = NULL; //현재위치
line* pre_present = NULL;//이전 문장
line* Top_line = NULL;//제일 위 프린트 문장
int x = 0, y = 0, screen_sum=0; //화면 속 커서 위치
unsigned int p_loc=0; //물리적 위치
int row=0, colum=0;// 행과 열
int string = -1, new_viva = 0, out_q = 0;//문자 줄 수, 새 파일인가, 나가기 여부
int end_line = 0, input_fixed =0, cursor_loc =1; // 출력라인의 마지막
char f_name[1024];//타입
char saving = 0;//저장
FILE* filename;
char* file_extension;

void make_struct(); //자료구조 생성 (중간 작업됨)
 // 커서 위치
//터미널 사이즈 가져오기 //완료
void open_file(FILE* filename); //파일 이름 있을 시 파일 불러오기(성공)
void savefile(); //파일 저장
void resize(); //매모리 재설정 (설정함)
void fixed_line();//마지막 라인
void input_word(int word);
void cursor_location();
void cursor_Up();
void cursor_Down();
void cursor_Left();
void cursor_Right();
void new_line();
void Backspace();
void Home();
void End();
void out_viva();
void Scroll_bar();

void make_struct() {
    line* liner = (line*)malloc(sizeof(line));
    if (liner == NULL) {
        printf("메모리 할당 실패");
        exit(1);
    }
    if (header == NULL) {
        liner->num = 0;
        liner->ptr1 = NULL;
        liner->ptr2 = NULL;
        liner->size = 1024;
        liner->text = (char*)malloc(liner->size * sizeof(char));
        memset(liner->text, '\0', liner->size);
        header = liner;
        present = liner;
        pre_present = liner;
        Top_line = liner;
        string++;
        return;
    }
    if (present->ptr2 != NULL) {
        liner->num = 0;
        liner->ptr1 = present;
        present = present->ptr2;
        liner->ptr2 = present;
        present->ptr1 = liner;
        present = liner->ptr1;
        liner->size = 1024;
        liner->text = (char*)malloc(liner->size * sizeof(char));
        memset(liner->text, '\0', liner->size);
        present->ptr2 = liner;
        pre_present = present;
        present = liner;
        string++;
        cursor_loc++;
        return;
    }
    liner->num = 0;
    liner->ptr1 = present;
    liner->ptr2 = NULL;
    liner->size = 1024;
    liner->text = (char*)malloc(liner->size * sizeof(char));
    memset(liner->text, '\0', liner->size);
    present->ptr2 = liner;
    pre_present = present;
    present = liner;
    string++;
    cursor_loc++;
    return;
    
}

void resize(){
    int plus_size = present -> size + 15;
    present->text = (char*)realloc(present->text, plus_size * sizeof(char));
    present->size = plus_size;
    return;
}

void open_file(FILE* filename){
    char oneline[1024];
    while (fgets(oneline, sizeof(oneline), filename) != NULL) {
        // 개행 문자 제거
        int len = strlen(oneline);
        if (len > 0 && oneline[len - 1] == '\n') {
            oneline[len - 1] = '\0';  // 개행 문자를 제거
            len--;
        }

        make_struct();
        if(len>present->size-1){
            present->size = len + 1;
            present->text = (char*)realloc(present->text,present->size * sizeof(char));
            if (present->text == NULL) {
                printf("메모리 재할당 실패");
                exit(1);
            }
        }
        strcpy(present->text, oneline);
        present->num = len;
        printw("%s\n", present->text);
    }
}

void fixed_line() {//아직 빈 공간에 흰색 안채워짐ㅋㅋㅋㅋ
    int print_len = 0, hidden_line=0;
    char lenght_storage[256];
    getmaxyx(stdscr, row, colum);
    start_color();
    attron(A_REVERSE);
    if (string == 0 || string == 1) {
        mvprintw(row - 2, 0, "[%s] - %d line", f_name, string);
        print_len = snprintf(lenght_storage, sizeof(lenght_storage), "[%s] - %d line", f_name, string);
    }
    else {
        mvprintw(row - 2, 0, "[%s] - %d lines", f_name, string);
        print_len = snprintf(lenght_storage, sizeof(lenght_storage), "[%s] - %d line", f_name, string);
    }
    hidden_line = snprintf(lenght_storage, sizeof(lenght_storage), "%s  %d/%d", file_extension, cursor_loc, string);
    mvprintw(row-2, colum -hidden_line, "%s  %d/%d", file_extension, cursor_loc, string);
    hidden_line = colum - hidden_line;
    for (int i = print_len; i < hidden_line; i++) {
        mvprintw(row - 2, i, " ");
    }
    attroff(A_REVERSE);
    if (input_fixed == 0) {
        mvprintw(row - 1, 0, "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    }
    refresh();
    move(x, y);
}

void cursor_location() {//현재 화면속 커서
    x = (screen_sum) / colum;
    y = (screen_sum) % colum;
}

void print() {
    Scroll_bar();
    clear();
    int l_y = 0;
    int offset = 0;
    for (int i = 0; i < row - 2; i++) {
        mvprintw(i, 0, "~");
    }
    line* printing = Top_line;
    offset = end_line * colum;
    while (printing != NULL && l_y < row - 2) {
        int text_len = printing -> num; 
        if (text_len == 0) {
            mvprintw(l_y, 0, " "); // 빈 줄 출력
            l_y++;
        }
        else {
            while (offset < text_len && l_y < row - 2) {
                // 문자열의 일부분을 출력
                mvprintw(l_y, 0, "%.*s", colum, printing->text + offset);

                offset += colum;
                l_y++;
            }
        }
        offset = 0;
        printing = printing->ptr2;
    }
    fixed_line();
    refresh();
}

void Scroll_bar() {//맨 위에서 연결리스트를 지웠을때가 문제...
    int location_y = (screen_sum) / colum;
    if (location_y == row - 2) {//출력되는 위치를 넘겼을때
        if (end_line < Top_line->num / colum) {//현재 출력되는 라인이 top_line의 마지막 라인이 아닐때
            end_line++;
            screen_sum = screen_sum - colum;
            cursor_location();
        }
        else {//작거나 같을때
            Top_line = Top_line->ptr2;
            end_line = 0;
            screen_sum = screen_sum - colum;
            cursor_location();
        }
    }
    else if (location_y < 0) {//맨 위 출력에서 올라갈때
        end_line--;
        if (end_line < 0) {//Top_line이 넘어갈때
            Top_line = Top_line->ptr1;
            end_line = Top_line->num / colum;
            screen_sum = screen_sum + colum;
            cursor_location();
        }
        else {//end_line이 0이거나 클때
            screen_sum = screen_sum + colum;
            cursor_location();
        }
    }
    else {
        if (screen_sum < 0) {
            end_line--;
            if (end_line < 0) {//Top_line이 넘어갈때
                Top_line = Top_line->ptr1;
                end_line = Top_line->num / colum;
                screen_sum = screen_sum + colum;
            }
            else {//end_line이 0이거나 클때
                screen_sum = screen_sum + colum;
            }
        }
        cursor_location();
    }
}

void cursor_Up() {//완료!
    //같은 배열의 맨 위이거나 그냥 올라가거나
    int calc = 0, number = 0;
    if (header == pre_present) {
        if (header == present) {//헤더가 present인가
            if (p_loc / colum == 0) {//헤더의 맨 윗줄 인가
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
            else {//헤더의 맨 윗줄이 아닐때
                p_loc = p_loc - colum;
                screen_sum = screen_sum - colum;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
        else {//present가 헤더는 아님
            if (p_loc / colum == 0) {//줄 바꿈 필요
                present = present->ptr1;
                pre_present = present;
                calc = p_loc % colum;
                if (calc >= present->num % colum) {//x값보다 num의 위치가 작거나 같을때
                    p_loc = present->num;
                    number = calc - (present->num % colum);
                    screen_sum = (screen_sum - colum) - number;
                    cursor_loc--;
                    cursor_location();
                    print();
                    move(x, y);
                    refresh();
                }
                else {//x 값보다 num의 위치가 클때
                    number = (present->num % colum) - calc;
                    p_loc = present->num - number;
                    screen_sum = screen_sum - colum;
                    cursor_loc--;
                    cursor_location();
                    print();
                    move(x, y);
                    refresh();
                }
            }
            else {//줄바꿈 필요없음
                p_loc = p_loc - colum;
                screen_sum = screen_sum - colum;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
    }
    else {//헤더와 관련 없는 줄일때
        if (p_loc / colum == 0) {//줄바꿈 필요
            pre_present = pre_present->ptr1;
            present = present->ptr1;
            calc = p_loc % colum;
            if (calc >= present->num % colum) {//x값보다 num의 위치가 작거나 같을때
                number = calc - (present->num % colum);
                p_loc = present->num;
                screen_sum = (screen_sum - colum) - number;
                cursor_loc--;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
            else {//x 값보다 num의 위치가 클때
                number = (present->num % colum) - calc;
                p_loc = present->num - number;
                screen_sum = screen_sum - colum;
                cursor_loc--;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
        else {//줄바꿈 필요없음
            p_loc = p_loc - colum;
            screen_sum = screen_sum - colum;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
    }
}
void cursor_Down() {//완료
    //같은 배열의 마지막이거나 그냥 내려가기나 마지막줄의 마지막이냐
    int calc = 0, number = 0;
    if (present->ptr2 == NULL) {//연결리스트 이동 불가능
        if (p_loc / colum == present->num / colum) {//마지막의 마지막줄
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {
            calc = p_loc + colum;
            if (calc <= present->num) {//x가 작거나 같을때
                p_loc = calc;
                screen_sum = screen_sum + colum;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
            else {//x가 클때
                p_loc = present->num;
                number = calc - present->num;
                screen_sum = (screen_sum + colum) - number;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
    }
    else {//연결리스트 이동 가능
        if (p_loc / colum == present->num / colum) {//연결리스트 이동
            if (header == pre_present && header == present) {
                pre_present = pre_present;
            }
            else {
                pre_present = pre_present->ptr2;
            }
            present = present->ptr2;
            calc = p_loc % colum;
            if (calc >= present->num) {//x가 크거나 같을때
                number = calc - present->num;
                p_loc = present->num;
                screen_sum = (screen_sum + colum) - number;
                cursor_loc++;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
            else {//x가 작을때//앤 잘됨
                p_loc = calc;
                screen_sum = screen_sum + colum;
                cursor_loc++;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
        else {//배열간의 이동
            calc = p_loc / colum;
            if (calc == (present->num / colum) -1) {//배열의 마지막 줄로 갈때
                calc = p_loc % colum;
                if (calc >= (present->num % colum)) {
                    p_loc = present->num;
                    number = calc - (present->num % colum);
                    screen_sum = (screen_sum + colum) - number;
                    cursor_location();
                    print();
                    move(x, y);
                    refresh();
                }
                else {
                    p_loc = p_loc + colum;
                    screen_sum = screen_sum + colum;
                    cursor_location();
                    print();
                    move(x, y);
                    refresh();
                }
            }
            else {
                p_loc = p_loc + colum;
                screen_sum = screen_sum + colum;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
    }
}

void cursor_Left() {//완료
    //같은 배열이거나 새배열 이동이거나 header거나
    if (p_loc == 0) {//배열의 맨앞
        if (present == header) {//첫줄 맨앞
            screen_sum = 0;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {//첫줄아님
            if (pre_present != header) {
                pre_present = pre_present->ptr1;
            }
            present = present->ptr1;
            p_loc = present->num;
            screen_sum = screen_sum - (colum - (present->num % colum));
            cursor_loc--;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
    }
    else {//맨앞이 아닌경우
        p_loc -= 1;
        screen_sum -= 1;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
}

void cursor_Right() {//완료
    //같은 배열이거나 새배열 이동이거나 header거나
    if (p_loc == present->num) {
        if (present->ptr2 == NULL) {//마지막줄 맨 마지막
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {//마지막 줄이 아닌 맨 마지막
            pre_present = present;
            present = present->ptr2;
            p_loc = 0;
            screen_sum = (screen_sum / colum + 1) * colum;
            cursor_loc++;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
    }
    else {//배열 사이
        p_loc++;
        screen_sum++;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
}

void input_word(int word) {//완료
    if (present->num > present->size - 1) {
        resize();
    }if (p_loc == 0) {  // 문자열 맨 앞에 삽입
        // 이미 문자가 있는 경우, 한 칸씩 뒤로 밀어야 함
        if (present->num > 0) {
            for (int i = present->num; i > 0; i--) {
                present->text[i] = present->text[i - 1];
            }
        }
        present->text[0] = word;
        present->num++;
        p_loc++;
        screen_sum++;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
    else if (p_loc < present->num) {//중간 삽입 및 맨 앞 삽입
        for (int i = present->num; i >= p_loc; i--) {
            present->text[i] = present->text[i-1];
        }//한칸씩 이동
        present->text[p_loc] = word;
        p_loc++;
        present->num++;
        screen_sum++;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
    else {//맨뒤 작성
        present->text[p_loc] = word;
        p_loc++;
        present->num++;
        screen_sum++;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
    out_q = 1;
    return;
}

void Backspace() {//완료
    unsigned int calc = 0;
    unsigned int re_size = 0;
    if (pre_present == header) {
        if (present == header) {//맨 첫줄
            if (p_loc == 0) {//맨 처음
                cursor_location();
                clear();
                print();
                move(x, y);
                refresh();
            }
            else if (p_loc == present->num) {//문장의 마지막 문자
                present->text[p_loc - 1] = '\0';
                p_loc = p_loc - 1;
                present->num = present->num - 1;
                screen_sum = screen_sum - 1;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }
            else {//문장 사이
                calc = present->num;
                for (int i = p_loc; i <= present->num; i++) {
                    present->text[i - 1] = present->text[i];
                }
                present->text[calc - 1] = '\0';
                present->num = present->num - 1;
                p_loc = p_loc - 1;
                screen_sum = screen_sum - 1;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }
        }
        else {//present가 두번째 줄일때
            if (p_loc == 0) {//맨 처음 문자보다 앞일때
                if (present->num == 0) {//빈 문장일때
                    line* del = present;
                    if (present->ptr2 != NULL) {
                        line* down = present->ptr2;
                        present = present->ptr1;
                        present->ptr2 = down;
                        down->ptr1 = present;
                    }
                    else {
                        pre_present->ptr2 = NULL;
                        present = present->ptr1;
                    }
                    if (Top_line == del) {
                        Top_line = present;
                    }
                    free(del);
                    p_loc = present->num;
                    screen_sum = present->num;
                    string--;
                    cursor_loc--;
                    cursor_location();
                    move(x, y);
                    clear();
                    print();
                    refresh();
                }
                else {//두번째 줄 일때
                    line* del = present;
                    if (present->ptr2 != NULL) {
                        line* down = present->ptr2;
                        present = present->ptr1;
                        present->ptr2 = down;
                        down->ptr1 = present;
                    }
                    else {
                        pre_present->ptr2 = NULL;
                        present = present->ptr1;
                    }
                    re_size = del->num + present->num + 1;
                    if (present->size < re_size) {
                        present->text = (char*)realloc(present->text, re_size * sizeof(char));
                    }
                    p_loc = present->num;
                    present->text[p_loc] = '\0';
                    strcpy(present->text+p_loc, del->text);
                    present->num = present->num + del->num;
                    screen_sum = p_loc;
                    if (Top_line == del) {
                        Top_line = present;
                    }
                    free(del);
                    string--;
                    cursor_loc--;
                    cursor_location();
                    move(x, y);
                    clear();
                    print();
                    refresh();
                }
            }
            else if (p_loc == present->num) {//문장의 마지막 문자
                present->text[p_loc - 1] = '\0';
                p_loc = p_loc - 1;
                present->num = present->num - 1;
                screen_sum = screen_sum - 1;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }
            else {//문장 사이
                calc = present->num;
                for (int i = p_loc; i <= present->num; i++) {
                    present->text[i - 1] = present->text[i];
                }
                present->text[calc - 1] = '\0';
                present->num = present->num - 1;
                p_loc = p_loc - 1;
                screen_sum = screen_sum - 1;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }
        }
    }
    else {
        if (p_loc == 0) {//맨 앞 문장일때
            if (present->num == 0) {//빈 문장 일때
                line* del = present;
                if (present->ptr2 != NULL) {
                    line* down = present->ptr2;
                    pre_present = pre_present->ptr1;
                    present = present->ptr1;
                    present->ptr2 = down;
                    down->ptr1 = present;
                }
                else {
                    pre_present = pre_present->ptr1;
                    present = present->ptr1;
                    present->ptr2 = NULL;
                }
                if (Top_line == del) {
                    Top_line = present;
                }
                free(del);
                calc = present->num % colum;
                p_loc = present->num;
                screen_sum = (screen_sum - colum) + calc;
                string--;
                cursor_loc--;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }else{//빈 문장이 아닐때
                line* del = present;
                if (present->ptr2 != NULL) {
                    line* down = present->ptr2;
                    pre_present = pre_present->ptr1;
                    present = present->ptr1;
                    present->ptr2 = down;
                    down->ptr1 = present;
                }
                else {
                    pre_present = pre_present->ptr1;
                    present = present->ptr1;
                    present->ptr2 = NULL;
                }
                re_size = del->num + present->num + 1;
                if (present->size < re_size) {
                    present->text = (char*)realloc(present->text, re_size * sizeof(char));
                }
                p_loc = present->num;
                calc = present->num % colum;
                present->text[p_loc] = '\0';
                strcpy(present->text + p_loc, del->text);
                present->num = present->num + del->num;
                screen_sum = (screen_sum - colum) + calc;
                if (Top_line == del) {
                    Top_line = present;
                }
                free(del);
                string--;
                cursor_loc--;
                cursor_location();
                move(x, y);
                clear();
                print();
                refresh();
            }
        }
        else if(p_loc == present->num){//배열의 마지막 단어
            present->text[p_loc - 1] = '\0';
            p_loc = p_loc - 1;
            present->num = present->num - 1;
            screen_sum = screen_sum - 1;
            cursor_location();
            move(x, y);
            clear();
            print();
            refresh();
        }
        else {//배열과 배열 사이
            calc = present->num;
            for (int i = p_loc; i <= present->num; i++) {
                present->text[i - 1] = present->text[i];
            }
            present->text[calc - 1] = '\0';
            present->num = present->num - 1;
            p_loc = p_loc - 1;
            screen_sum = screen_sum - 1;
            cursor_location();
            move(x, y);
            clear();
            print();
            refresh();
        }
    }
    out_q = 1;
}

void Home() {//완료
    int calc = 0;
    if (p_loc % colum == 0) {
        cursor_location();
        clear();
        print();
        move(x, y);
        refresh();
    }
    else {
        calc = p_loc % colum;
        p_loc = p_loc - calc;
        screen_sum = screen_sum - calc;
        cursor_location();
        clear();
        print();
        move(x, y);
        refresh();
    }
}

void End() {//완료
    int calc = 0, ex =0;
    if (p_loc % colum == colum - 1) {
        cursor_location();
        clear();
        print();
        move(x, y);
        refresh();
    }
    else {
        calc = (colum - 1) - (p_loc%colum);
        ex = p_loc + calc;
        if (present->num > ex) {
            p_loc = ex;
            screen_sum = screen_sum + calc;
            cursor_location();
            clear();
            print();
            move(x, y);
            refresh();
        }
        else {
            calc = present->num - p_loc;
            p_loc = present->num;
            screen_sum = screen_sum + calc;
            cursor_location();
            clear();
            print();
            move(x, y);
            refresh();
        }
    }
}

void new_line() {
    if (p_loc == 0) {//맨 앞 배열
        if (present == header) {//만일 present가 그냥 있을때
            line* liner = (line*)malloc(sizeof(line));
            if (liner == NULL) {
                printf("메모리 할당 실패");
                exit(1);
            }
            liner->ptr2 = present;
            liner->ptr1 = NULL;
            liner->num = 0;
            liner->size = 1024;
            liner->text = (char*)malloc(liner->size * sizeof(char));
            memset(liner->text, '\0', liner->size);
            Top_line = liner;
            present->ptr1 = liner;
            header = liner;
            pre_present = liner;
            screen_sum = screen_sum + colum;
            string++;
            cursor_loc++;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {
            line* liner = (line*)malloc(sizeof(line));
            if (liner == NULL) {
                printf("메모리 할당 실패");
                exit(1);
            }
            liner->ptr1 = pre_present;
            liner->ptr2 = present;
            liner->num = 0;
            liner->size = 1024;
            liner->text = (char*)malloc(liner->size * sizeof(char));
            memset(liner->text, '\0', liner->size);
            present->ptr1 = liner;
            pre_present->ptr2 = liner;
            pre_present = liner;
            screen_sum = screen_sum + colum;
            string++;
            cursor_loc++;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
    }else if (p_loc<present->num) {//중간 삽입일때
        char arr[1024];
        int number = present->num;
        memcpy(arr, present->text + p_loc, (number - p_loc) * sizeof(char));
        memset(present->text + p_loc, '\0', (number - p_loc) * sizeof(char));
        present->num = p_loc;
        make_struct();
        present->num = number - p_loc;
        strcpy(present->text, arr);
        present->text[present->num] = '\0';
        p_loc = 0;
        //스크린상 x 좌표 넣어야함
        screen_sum = (screen_sum / colum + 1) * colum;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
    else {//그냥 새 배열
        make_struct();
        screen_sum = (screen_sum / colum + 1) * colum;
        p_loc = 0;
        cursor_location();
        print();
        move(x, y);
        refresh();
    }
    out_q = 1;
    return;
}

void savefile() {
    if (saving == 0) {//새파일 저장은 됨 but 파일 이름 입력 위치 이상함 
        line* save_line = (line*)malloc(sizeof(line));
        save_line = header;
        input_fixed = 1;
        print(); //맨 마지막에 빈 문장이도록 함
        mvgetnstr(row - 1, 0, f_name, sizeof(f_name) - 1);
        filename = fopen(f_name, "wb");
        if (filename == NULL) {
            printf("파일 생성 실패");
            return;
        }
        if (setvbuf(filename, NULL, _IOFBF, 1024) != 0) {
            perror("buffer 생성 실패");
            fclose(filename);
            return;
        }
        file_extension = strrchr(f_name, '.');
        if (file_extension == NULL) {
            file_extension = "no ft";
        }
        saving = 1;
        while (save_line!= NULL) {
            fputs(save_line->text, filename);
            fputc('\n', filename);
            save_line = save_line->ptr2;
        }
        fclose(filename);
    }
    else {//기존 파일
        line* save_line = (line*)malloc(sizeof(line));
        save_line = header;
        filename = fopen(f_name, "wb");
        if (filename == NULL) {
            printf("파일 생성 실패");
            return;
        }
        if (setvbuf(filename, NULL, _IOFBF, 1024) != 0) {
            perror("buffer 생성 실패");
            fclose(filename);
            return;
        }
        while (save_line != NULL) {
            fputs(save_line->text, filename);
            fputc('\n', filename);
            save_line = save_line->ptr2;
        }
        fclose(filename);
    }
    cursor_location();
    print();
    move(x, y);
    input_fixed = 0;
    out_q = 0;
    refresh();
}

void out_viva() {//내용 수정만 하면 될듯
    if (out_q == 1) {
        print();
        out_q = 0;
    }
    else {
        refresh();
        exit(0);//정상종료
    }
}

void page_Up() {
    line* paging = present;
    unsigned int page_line, page_eline;
    page_line = p_loc / colum;//현재 위치의 라인 위치
    page_eline = row - 2;
    for (int i = 0; i < page_eline; i++) {
        if (page_line > 0) {//연결리스트 옴릭 필요 없을때
            page_line--;
            p_loc = page_line * 120;
            Top_line = paging;
            end_line = page_line;
            screen_sum = 0;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {//연결리스트가 0이거나 이하면 움직여야함
            if (paging->ptr1 == NULL) {
                end_line = 0;
                present = paging;
                pre_present = paging;
                Top_line = paging;
                screen_sum = 0;
                p_loc = 0;
                cursor_location();
                print();
                move(x, y);
                refresh();
                return;
            }
            else {//널이 아닐때
                paging = paging->ptr1;
                Top_line = paging;
                page_line = paging->num / colum;
                end_line = page_line;
                present = paging;
                if (paging->ptr1 == NULL) {
                    pre_present = paging;
                }
                else {
                    pre_present = paging->ptr1;
                }
                p_loc = page_line * 120;
                screen_sum = 0;
                cursor_loc--;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
    }
}

void page_Down() {
    line* paging = present;
    unsigned int page_line, page_eline, page_end_line;
    page_line = p_loc / colum;//현재 위치의 라인 위치
    page_eline = row - 2;
    page_end_line = paging->num / colum;
    for (int i = 0; i < page_eline; i++) {
        if (page_line < page_end_line) {//연결리스트의 마지막 줄이 아닐때
            page_line++;
            p_loc = page_line * 120;
            Top_line = paging;
            end_line = page_line;
            screen_sum = 0;
            cursor_location();
            print();
            move(x, y);
            refresh();
        }
        else {//마지막 줄이면 연결리스트를 옮겨야함
            if (paging->ptr2 == NULL) {//더이상 옮기지 못할때
                end_line = paging->num / colum;
                page_line = end_line;
                present = paging;
                if (present->ptr1 == NULL) {
                    pre_present = paging;
                }
                else {
                    pre_present = paging->ptr1;
                }
                Top_line = paging;
                screen_sum = 0;
                p_loc = page_line * 120;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
            else {//옮기기가 가능할 때
                paging = paging->ptr2;
                present = paging;
                pre_present = paging->ptr1;
                Top_line = paging;
                end_line = 0;
                page_line = 0;
                p_loc = page_line * 120;
                screen_sum = 0;
                cursor_loc++;
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
    }
}


void find_word() {
    char* word_find;
    char find_word[256];//찾을 단어,
    int saved_ploc = 0, saved_screen = 0,saved_eline = 0, out_enter=0;
    int behavior, modify_word;
    int find_x = 0, find_y = row -1, F_position=0, sense =0;//find에서 사용할 커서 위치
    line* search_line = header;
    line* saved_line_print = Top_line;
    line* saved_present = present;
    input_fixed = 1;
    saved_ploc = p_loc;
    saved_screen = screen_sum;
    saved_eline = end_line;
    //맨 마지막에 빈 문장이도록 함
    while (1) {
        search_line = header;
        print();
        move(find_y, find_x);
        noecho();
        sense = getch();
        if (sense == Enter || sense == KEY_ENTER || sense == 13) {
            p_loc = saved_ploc;
            screen_sum = saved_screen;
            end_line = saved_eline;
            present = saved_present;
            Top_line = saved_line_print;
            input_fixed = 0;
            cursor_location();
            print();
            move(x, y);
            refresh();
            return;
        }
        else if (sense == Esc) {
            p_loc = saved_ploc;
            screen_sum = saved_screen;
            end_line = saved_eline;
            present = saved_present;
            Top_line = saved_line_print;
            input_fixed = 0;
            cursor_location();
            print();
            move(x, y);
            refresh();
            return;
        }
        else {
            ungetch(sense);
            echo();
            getstr(find_word);
            while (1) {
                if (out_enter == 1) {
                    out_enter = 0;
                    break;
                }
                word_find = strstr(search_line->text, find_word);
                if (word_find == NULL) {//찾는 단어가 이 배열에 없을때
                    if (search_line->ptr2 == NULL) {//마지막 줄이었을때
                        print();
                        break;
                    }
                    else {
                        search_line = search_line->ptr2;
                        print();
                        continue;
                    }
                }
                else {//찾는 단어가 배열에 있을때
                    F_position = word_find - search_line->text;
                    present = search_line;
                    Top_line = search_line;
                    end_line = F_position / colum;
                    p_loc = F_position;
                    screen_sum = F_position % colum;
                    cursor_location();
                    print();
                    attron(A_REVERSE);
                    mvprintw(x, y, "%s", find_word);
                    attroff(A_REVERSE);
                    mvprintw(row - 1, 0, "%s", find_word);
                    move(x, y);
                    refresh();
                    while(1){
                        behavior = getch();
                        if (behavior == KEY_RIGHT || behavior == R_Arrow) {
                            while (1) {
                                if (word_find != NULL) {
                                    word_find = strstr(word_find + strlen(find_word), find_word);
                                }

                                if (word_find == NULL) {
                                    if (search_line->ptr2 == NULL) { // 마지막 줄
                                        break;
                                    }
                                    else { // 다음 줄로 이동
                                        search_line = search_line->ptr2;
                                        word_find = strstr(search_line->text, find_word);
                                        if (word_find == NULL) {
                                            continue;
                                        }
                                    }
                                }
                                F_position = word_find - search_line->text;
                                present = search_line;
                                Top_line = search_line;
                                end_line = F_position / colum;
                                p_loc = F_position;
                                screen_sum = F_position % colum;
                                cursor_location();
                                print();
                                attron(A_REVERSE);
                                mvprintw(x, y, "%s", find_word);
                                attroff(A_REVERSE);
                                mvprintw(row - 1, 0, "%s", find_word);
                                move(x, y);
                                refresh();
                                break;
                            }
                        }
                        else if (behavior == KEY_LEFT || behavior == L_Arrow) {
                            while (1) {
                                if (word_find != NULL && word_find > search_line->text) {
                                    char* position = search_line->text;
                                    char* fine_preword = NULL;

                                    // 현재 줄에서 이전 위치 탐색
                                    while (position < word_find) {
                                        char* next_word = strstr(position, find_word);
                                        if (next_word == NULL || next_word >= word_find) {
                                            break;
                                        }
                                        fine_preword = next_word; // 이전 단어 위치 갱신
                                        position = next_word + 1;
                                    }

                                    if (fine_preword != NULL) {
                                        word_find = fine_preword; // 이전 위치로 이동
                                        F_position = word_find - search_line->text;
                                        p_loc = F_position;
                                        screen_sum = F_position % colum;
                                        end_line = F_position / colum;
                                        present = search_line;
                                        Top_line = search_line;

                                        cursor_location();
                                        print();
                                        attron(A_REVERSE);
                                        mvprintw(x, y, "%s", find_word); // 강조 표시
                                        attroff(A_REVERSE);
                                        mvprintw(row - 1, 0, "%s", find_word);
                                        move(x, y);
                                        refresh();
                                        break;
                                    }
                                }
                                if (search_line->ptr1 == NULL) {
                                    break; 
                                }
                                else {
                                    search_line = search_line->ptr1;
                                    word_find = strstr(search_line->text, find_word);

                                    if (word_find == NULL) {
                                        continue; 
                                    }
                                    else {
                                        char* final_find = NULL;
                                        char* this = search_line->text;

                                        while ((this = strstr(this, find_word)) != NULL) {
                                            final_find = this;
                                            this += strlen(find_word);
                                        }

                                        if (final_find != NULL) {
                                            word_find = final_find;
                                            F_position = word_find - search_line->text;
                                            p_loc = F_position;
                                            screen_sum = F_position % colum;
                                            end_line = F_position / colum;
                                            present = search_line;
                                            Top_line = search_line;

                                            cursor_location();
                                            print();
                                            attron(A_REVERSE);
                                            mvprintw(x, y, "%s", find_word); 
                                            attroff(A_REVERSE);
                                            mvprintw(row - 1, 0, "%s", find_word);
                                            move(x, y);
                                            refresh();
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else if (behavior == Enter || behavior == KEY_ENTER || behavior == 13) {
                            
                            while (1) {
                                cursor_location();
                                mvprintw(row - 1, 0, "%s", find_word);
                                move(x, y);
                                refresh();
                                modify_word = getch();
                                if (modify_word == KEY_BACKSPACE || modify_word == Back_Space) {
                                    Backspace();
                                    cursor_location();
                                    print();
                                    mvprintw(row - 1, 0, "%s", find_word);
                                    move(x, y);
                                    refresh();
                                }
                                else if (modify_word >= 32 && modify_word <= 126) {
                                    input_word(modify_word);
                                    cursor_location();
                                    print();
                                    mvprintw(row - 1, 0, "%s", find_word);
                                    move(x, y);
                                    refresh();
                                }
                                else if (modify_word == Enter || modify_word == KEY_ENTER || modify_word == 13) {
                                    out_enter = 1;
                                    break;
                                }
                                else if (modify_word == KEY_LEFT || modify_word == L_Arrow || modify_word== 4 ) {
                                    cursor_Left();
                                    cursor_location();
                                    print();
                                    mvprintw(row - 1, 0, "%s", find_word);
                                    move(x, y);
                                    refresh();
                                }
                                else if (modify_word == KEY_RIGHT || modify_word == R_Arrow ||modify_word == 5) {
                                    cursor_Right();
                                    cursor_location();
                                    print();
                                    mvprintw(row - 1, 0, "%s", find_word);
                                    move(x, y);
                                    refresh();
                                }
                            }
                            break;
                        }
                        else if (behavior == Esc) {
                            p_loc = saved_ploc;
                            screen_sum = saved_screen;
                            end_line = saved_eline;
                            present = saved_present;
                            Top_line = saved_line_print;
                            input_fixed = 0;
                            cursor_location();
                            print();
                            mvprintw(row - 1, 0, "%s", find_word);
                            move(x, y);
                            refresh();
                            return;
                        }
                        else {
                            continue;
                        }
                    }
                }
            }
        }

    }
    input_fixed = 0;
}


int main(int argc, char * argv[]){
    int word;
    initscr();
    keypad(stdscr, TRUE);
    raw();
    if(argc<2){
        strncpy(f_name, "no name", sizeof(f_name));
        saving = 0;
        out_q = 1;
        file_extension = "no ft";
        make_struct();
        clear();
        fixed_line();
        print();
        new_viva = 1;
    }
    else{
        strncpy(f_name, argv[1], sizeof(f_name));
        filename = fopen(argv[1],"r");
        if(filename == NULL){
            printf("파일 생성 실패");
            return 0;
        }
        if(setvbuf(filename, NULL, _IOFBF, 1024) != 0){
            perror("buffer 생성 실패");
            fclose(filename);
            return 0;
        }
        file_extension = strrchr(f_name, '.');
        if (file_extension == NULL) {
            file_extension = "no ft";
        }
        clear();
        open_file(filename);
        fclose(filename);
        saving = 1;
        Top_line = header;
        present = header;
        pre_present = header;
        p_loc = 0;
        string++;
        out_q = 0;
        cursor_loc=0;
        fixed_line();
        print();
        //파일 불러오기
    }
    while(1){
        getmaxyx(stdscr, row, colum); //row = 30, colum = 120;
        if (new_viva == 1) {
            x = colum / 3;
            y = row / 3;
            mvprintw(y, x, "Visual Text editor -- version 0.0.1");
            cursor_location();
            new_viva = 0;
            string++;
        }
        move(x, y);
        refresh();
        noecho();
        word = getch();
        clear();
        if (word == KEY_UP || word == U_Arrow) {
            cursor_Up();
        }
        else if (word == KEY_DOWN||word == D_Arrow) {
            cursor_Down();
        }
        else if (word == KEY_LEFT||word == L_Arrow) {
            cursor_Left();
        }
        else if (word == KEY_RIGHT||word == R_Arrow) {
            cursor_Right();
        }
        else if (word == KEY_BACKSPACE || word == Back_Space) {
            Backspace();
        }
        else if (word == Enter || word == KEY_ENTER || word == 13) {
            new_line();
        }
        else if (word == KEY_HOME || word == Numpad_Home) {
            Home();
        }
        else if (word == KEY_END || word == Numpad_End){
            End();
        }
        else if (word == Numpad_PgUp || word == KEY_PPAGE) {
            page_Up();
        }
        else if (word == Numpad_PgDn || word == KEY_NPAGE) {
            page_Down();
        }
        else if (word >= 32 && word <= 126) {
            input_word(word);
        }
        else {
            if (word == CtrlS) {
                echo();
                savefile();
            }
            else if (word == CtrlQ) {
                out_viva();
            }
            else if (word == CtrlF) {
                echo();
                find_word();
            }
            else {
                cursor_location();
                print();
                move(x, y);
                refresh();
            }
        }
        
    }
}