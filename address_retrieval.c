#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>

#define ADVANCED 1 //発展課題（絞り込み検索）に対応する場合は1に変更

#define DATAFILE "data_utf.csv" //data_utf.csvかdata_sjis.csvに変更
#define CLEN 9 //郵便番号の最大バイト長
#define ALEN 200 //住所欄の最大バイト長
#define MAX_SIZE 200000//住所録中の住所数の最大数

long LINE;

int mode; //検索モード 0:なし，1:郵便番号検索，2:文字列検索
int refine_flag; //絞り込み検索の有無 0:なし，1:あり
char query[ALEN]; //検索クエリ（郵便番号or文字列）

int num = 0, n = 0;


struct address {
      char code[CLEN+1];
      char pref[ALEN+1];
      char city[ALEN+1];
      char town[ALEN+1];
};
struct address *record;

struct address_rec {
    int code;
    char address_record[3*ALEN+3];
};
struct address_rec *ad_record;

//住所データファイルを読み取り，配列に保存
void scan(){
  FILE *fp;
  char code[CLEN+1],pref[ALEN+1],city[ALEN+1],town[ALEN+1];
  long line = 0;
  record = (struct address *)malloc(MAX_SIZE * sizeof(struct address));
  //datasizeの計算
  if ((fp = fopen(DATAFILE, "r")) == NULL) {
    fprintf(stderr,"error:cannot read %s\n",DATAFILE);
    exit(-1);
  }
  while(fscanf(fp, "%*[^,],%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%[^,],%[^,],%[^,],%*s",code,pref,city,town) != EOF ){
      
      for (int i = 1; code[i] != '"'; i++) record[line].code[i-1] = code[i];
      for (int i = 1; pref[i] != '"'; i++) record[line].pref[i-1] = pref[i];
      for (int i = 1; city[i] != '"'; i++) record[line].city[i-1] = city[i];
      for (int i = 1; town[i] != '"'; i++) record[line].town[i-1] = town[i];
      
      line++;
  }
    LINE = line;
  printf("%ld行の住所があります\n",line);
  fclose(fp);
}

void preprocess(){
  return;
}

void swap(int i, int j){
    int temp = ad_record[i].code;
    ad_record[i].code = ad_record[j].code;
    ad_record[j].code = temp;
    
    if (strcmp(ad_record[i].address_record, ad_record[j].address_record) != 0) {
        char t[3*ALEN+3];
        strcpy(t, ad_record[i].address_record);
        strcpy(ad_record[i].address_record, ad_record[j].address_record);
        strcpy(ad_record[j].address_record, t);
    }
}

int partition (int low, int high){
    int pivot = ad_record[high].code;
    int i = (low - 1);
    for (int j = low; j < high ; j++) {
        if (ad_record[j].code < pivot){
            i++;
            swap(i,j);
        }
    }
    swap(i+1, high);
    i++;
    return i;
}
void quickSort(int low, int high){
    if (low < high) {
        int pi = partition(low, high);
        quickSort(low, pi - 1);
        quickSort(pi + 1, high);
    }
}

double diff_time(clock_t t1, clock_t t2){
  return (double) (t2-t1)/CLOCKS_PER_SEC;
}

//初期化処理
void init(){
  clock_t t1,t2;
  t1 = clock();
  scan();
  preprocess();
  printf("Done initilization\n");
  t2 = clock();
  printf("\n### %f sec for initialization. ###\n",diff_time(t1,t2));
}

//郵便番号による住所検索．検索結果を出力．
void code_search(){
    long i = 0;
    while (i < LINE) {
        if (!strcmp(record[i].code,query)) {
            printf("%s:%s%s%s\n", record[i].code,
                                  record[i].pref,
                                  record[i].city,
                                  record[i].town);
        }
        i++;
    }
  return;
}

//文字列による住所検索．検索結果を出力．
void address_search(){
    char *p_town, *p_city, *p_pref, *ptr;
    ad_record = malloc(sizeof(struct address_rec));
    for (long i = 0; i < LINE ; i++) {
        p_town = strstr(record[i].town,query);
        p_city = strstr(record[i].city,query);
        p_pref = strstr(record[i].pref,query);
        if (p_pref || p_city || p_town) {
            ad_record[num].code = 0;
            for (int j = 0; record[i].code[j] != '\0' ; j++) {
                ad_record[num].code = ad_record[num].code * 10 + (record[i].code[j] - '0');
            }
            sprintf(ad_record[num].address_record,"%s%s%s",record[i].pref,
                                                           record[i].city,
                                                           record[i].town);
            num++;
            ad_record = realloc(ad_record, (num+1) * sizeof(struct address_rec));
        }
    }
    quickSort(0,num-1);
    for (int i = 0; i < num ; i++) {
        if (ad_record[i].code != 0) {
            printf("%d:%s\n",ad_record[i].code,
                             ad_record[i].address_record);
        }
    }
  return;
}

//絞り込み検索の実施
void refinement(){
    char *ptr;
    for (long i = 0; i < num ; i++) {
        ptr = strstr(ad_record[i].address_record,query);
        if (ptr) {
            printf("%d:%s\n",ad_record[i].code,
                             ad_record[i].address_record);
            ad_record[n].code = ad_record[i].code;
            strcpy(ad_record[n].address_record,ad_record[i].address_record);
            n++;
        }
    }
    ad_record = realloc(ad_record, (num+1) * sizeof(struct address_rec));
    num = 0;
  return;
}

void input(){
  printf("\n"
     "#########Top Menu#########\n"
     "# Search by postal code: 1\n"
     "# Search by address    : 2\n"
     "# Exit                 : 0\n"
     "> ");
  scanf("%d", &mode);
  if(mode == 1){
    printf("Postal code > ");
    scanf("%s", query);
  }else if(mode == 2){
    printf("Search String > ");
    scanf("%s", query);
  }
}

//絞り込み検索の有無を確認
void re_input(){
  printf("\n"
     "# Continue Searching: 1\n"
     "# Return to Top Menu: 0\n"
     "> ");
  scanf("%d", &refine_flag);
  if(refine_flag == 1){
    printf("String for Refinement> ");
    scanf("%s", query);
  }
  return;
}

//クエリへの応答
void respond(){
  clock_t t1,t2;
  mode = 1;
  while(1){
    input();
    if(mode == 1){
      t1 = clock();
      code_search();
      t2 = clock();
      printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
    }
    else if(mode == 2){
      t1 = clock();
      address_search();
      t2 = clock();
      printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
      if(!ADVANCED) continue;
      while(1){
        re_input();
        if(refine_flag == 0) break;
        t1 = clock();
        refinement();
        t2 = clock();
        printf("\n### %f sec for search. ###\n", diff_time(t1,t2));
      }
    }
    else break;
  }
}


int main()
{
  init();
  respond();
  return 0;
}
