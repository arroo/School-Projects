#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct record {
    char * name,
         * disease,
         * postal_code,
         * phone,
         * dob;
    int male, good_enough;
    struct record *anoned; // for each record, store an extra one with the anonimized info
};

print_anoned(struct record *in, int n, char filename[]) { // print the current stage's
                                                          // version of the anonymized
                                                          // info
    int i;
    FILE * fp;
    struct record * cur;
    if ((fp = fopen(filename,"w+"))==0){
        fprintf(stderr,"could not open %s file for output\n",filename);
        exit(-1);
    }
    
    fprintf(fp,"Name,Gender,Date of birth,Disease,Postal code,Telephone\n");
    for (i = 0; i < n; i++){
        cur = in[i].anoned;
        fprintf(fp,
                "%s,%c,%s,%s,%s,%s\n",
                cur->name,
                (cur->male==1?'M':'F'),
                cur->dob,
                cur->disease,
                cur->postal_code,
                cur->phone
               );
    }
    
    fclose(fp);
}

void stage2fix(struct record * in) { // anonymize the day in the DOB
    strcpy(strchr(in->dob+5, '-')+1,"*");
}

void stage3fix(struct record * in) { // anonymize the month in the DOB
    strcpy(in->dob+5,"*-*");
}

void stage4fix(struct record * in) { // anonymize the last 3 character
                                     // of the postal code
    strcpy(in->postal_code+4,"*");
}

void stage5fix(struct record * in) { // these records are not k-anonymizable
                                     // using these techniques alone
    in->good_enough = 0;
}

void next_stage(struct record * in, int n, void(*fix)(struct record *)) {
  // this is not a very efficient function if there is more than one
  // stage but it passed quickly enough on the given virtual machines
    struct bucket {
        struct record *me;
        struct bucket *next;
        
    };
    struct chain {
        struct bucket *bucket;
        struct chain *next;
    };
    int i;
    struct bucket *tmp,
                  *bfreer = 0;
    struct chain * root = 0,
           *cur,
           *cfreer = 0;
    if ((root = malloc(sizeof(struct chain))) == 0) {
        fprintf(stderr,"could not allocate chain\n");
        exit(-1);
    }
    root->next = 0;
    root->bucket = 0;
    if (n > 0) {
        if((root->bucket = malloc(sizeof(struct bucket))) == 0){
            fprintf(stderr,"could not allocate bucket\n");
            exit(-1);
        }
        root->bucket->next = 0;
        root->bucket->me = in[0].anoned;
    }
    
    // make chains of the same gender, dob, & postal code-having records
    for (i = 1; i < n; i++) {
        cur = root;
        if ((tmp = malloc(sizeof(struct bucket))) == 0) {
            fprintf(stderr,"could not allocate tmp bucket\n");
            exit(-1);
        }
        tmp->me = in[i].anoned;
        while (cur) {
            //looking at cur and in[i].anoned
            if (cur->bucket->me->male == in[i].anoned->male && // gender
                strcmp(cur->bucket->me->postal_code, in[i].anoned->postal_code) == 0 && // postal code
                strcmp(cur->bucket->me->dob,in[i].anoned->dob) == 0) { // dob
                // they are indistinguishable
                    tmp->next = cur->bucket;
                    cur->bucket = tmp;
                    break;
            }
            cur = cur->next;
        }
        if (cur) continue; // not a match
        if ((cur = malloc(sizeof(struct chain))) == 0) {
            fprintf(stderr,"could not allocate tmp chain\n");
            exit(-1);
        }
        cur->next = root;
        cur->bucket = tmp;
        tmp->next = 0;
        root = cur;
    }
    for (cur = root; cur; cur = cur->next){
        i = 0;
        for (tmp = cur->bucket; tmp; tmp = tmp->next) i++;
        if (i < 6/* the k in k-anonymous! */) {
            for(tmp = cur->bucket; tmp; tmp = tmp->next){
                fix(tmp->me);
            }
        }
    }
    cur = root;
    while (cur) {
        tmp = cur->bucket;
        while (tmp) {
            bfreer = tmp;
            tmp = tmp->next;
            free(bfreer);
        }
        cfreer = cur;
        cur = cur->next;
        free(cfreer);
    }
    root = 0;
}

void print_nonkanoned(struct record *in,int n) { // print every record that is not k-anonymized
    int i;
    struct record * cur;
    FILE * fp;
    if ((fp = fopen("non-kanonymized.csv","w+")) == 0) {
        fprintf(stderr,"could not open non-kanonymized.csv");
        exit(-1);
    }
    
    fprintf(fp,"Name,Gender,Date of birth,Disease,Postal code,Telephone\n");
    for (i = 0; i < n; i++){
        cur = &in[i];
        if (cur->anoned->good_enough == 0)
            fprintf(fp,
                    "%s,%c,%s,%s,%s,%s\n",
                    cur->name,
                    (cur->male==1?'M':'F'),
                    cur->dob,
                    cur->disease,
                    cur->postal_code,
                    cur->phone
                   );
    }
    

}



int main(int argc, char * argv[]) {
    int kanon = 6,
        i,
        j,
        k,
        curlen,
        blocksize = 1024,
        numrecords=-1; // it thinks there is one more than there is
    char filename[] = "Infectious_Disease_Record.csv",
         current,
         *block;
    struct record * records = 0;
    FILE * fp;
    
    if((block = malloc(blocksize*sizeof(char)))==0){
        fprintf(stderr,"could not allocate block\n");
        exit(-1);
    }
    
    if((fp = fopen(filename,"r"))==0){
        fprintf(stderr,"could not open file %s\n",filename);
        exit(-1);
    }
    
    // setup
    while ((current = fgetc(fp)) != EOF){
        if (current == '\n') numrecords++;
    }
    rewind(fp);
    records = malloc(numrecords*sizeof(struct record));
    if (records == 0) {
        fprintf(stderr,"could not allocate block\n");
        exit(-1);
    }
    while ((current = fgetc(fp)) != '\n'); // the first line is just the headers
    i = 0;
    while ((current = fgetc(fp)) != EOF){ // read in the records
        // reset block;
        memset(block,0,blocksize);
        // read line
        j = 1;
        block[0] = current;
        while ((current = fgetc(fp)) != '\n') {
            if (j >= blocksize){
                blocksize += blocksize; // doubling is a good strategy
                block = realloc(block,blocksize*sizeof(char));
            }
            block[j] = current;
            j++;
        }
        // parse name
        curlen = 0;
        for (k = 0; block[k] != ','; k++) curlen++;
        if ((records[i].name = malloc((curlen+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth name",i);
            exit(-1);
        }
        memset(records[i].name,0,curlen+1);
        for (k = 0; block[k] != ','; k++) {
            records[i].name[k] = block[k];
        }
        
        //parse sex
        records[i].male = (block[curlen+1] == 'M' || block[curlen+1] == 'm' ? 1 : 0);
        
        //parse dob
        j = curlen + 3; // sex was just one character
        curlen = 0;
        for (k = j; block[k] != ','; k++) curlen++;
        if ((records[i].dob = malloc((curlen+1)*sizeof(char))) == 0){
            fprintf(stderr,"error: could not allocate %dth dob",i);
            exit(-1);
        }
        memset(records[i].dob,0,curlen+1);
        for (k = 0; block[k+j] != ','; k++) {
            records[i].dob[k] = block[k + j];
        }
        
        //parse disease
        j += curlen + 1;
        curlen = 0;
        for (k = j; block[k] != ','; k++) curlen++;
        if ((records[i].disease = malloc((curlen+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth disease",i);
            exit(-1);
        }
        memset(records[i].disease,0,curlen+1);
        for (k = 0; block[k + j] != ','; k++) {
            records[i].disease[k] = block[k + j];
        }
        
        //parse postal code
        j += curlen + 1;
        curlen = 0;
        for (k = j; block[k] != ','; k++) curlen++;
        if ((records[i].postal_code = malloc((curlen+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth postal code",i);
            exit(-1);
        }
        memset(records[i].postal_code,0,curlen+1);
        for (k = 0; block[k + j] != ','; k++) {
            records[i].postal_code[k] = block[k + j];
        }
        
        //parse phone number
        j += curlen + 1;
        curlen = 0;
        for (k = j; block[k] != '\0'; k++) curlen++;
        if ((records[i].phone = malloc((curlen+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth phone number",i);
            exit(-1);
        }
        memset(records[i].phone,0,curlen+1);
        for(k = 0; block[k + j] != '\0'; k++) {
            records[i].phone[k] = block[k + j];
        }
        // setup the anonymizing part along with stage 1 processing
        if ((records[i].anoned = malloc(sizeof(struct record))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized part",i);
            exit(-1);
        }
        if ((records[i].anoned->name = malloc(2*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized name",i);
            exit(-1);
        }
        strcpy(records[i].anoned->name,"*");
        if ((records[i].anoned->dob = malloc((strlen(records[i].dob)+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized dob",i);
            exit(-1);
        }
        strcpy(records[i].anoned->dob,records[i].dob);
        if ((records[i].anoned->disease = malloc((strlen(records[i].disease)+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized disease",i);
            exit(-1);
        }
        strcpy(records[i].anoned->disease,records[i].disease);
        if ((records[i].anoned->postal_code = malloc((strlen(records[i].postal_code)+1)*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized postal code",i);
            exit(-1);
        }
        strcpy(records[i].anoned->postal_code,records[i].postal_code);
        if ((records[i].anoned->phone = malloc(2*sizeof(char))) == 0) {
            fprintf(stderr,"error: could not allocate %dth anonymized phone number",i);
            exit(-1);
        }
        strcpy(records[i].anoned->phone,"*");
        records[i].anoned->male = records[i].male;
        records[i].anoned->anoned = 0;
        records[i].anoned->good_enough = 1; // innocent until proven guilty
        i++;
    }
    fclose(fp);
    free(block);
    // since stage 1 anonymizing was completed in setup, just print the anonymized values
    // to the first 'checkpoint' file
    print_anoned(records,numrecords,"anonymized.csv.1");
    
    // stage 2 processing & printing
    next_stage(records,numrecords,&stage2fix);
    print_anoned(records,numrecords,"anonymized.csv.2");
    
    // stage 3 processing & printing
    next_stage(records,numrecords,&stage3fix);
    print_anoned(records,numrecords,"anonymized.csv.3");
    
    // stage 4 processing & printing
    next_stage(records,numrecords,&stage4fix);
    print_anoned(records,numrecords,"anonymized.csv");
    
    // I can reuse next_stage because it takes in a function pointer
    // for all of the non-k-anonymized records
    next_stage(records,numrecords,&stage5fix);
    print_nonkanoned(records,numrecords);
    
    // now to free everything
    for (i = 0; i < numrecords; i++){
        free(records[i].name);
        free(records[i].dob);
        free(records[i].phone);
        free(records[i].postal_code);
        free(records[i].disease);
        //
        free(records[i].anoned->name);
        free(records[i].anoned->dob);
        free(records[i].anoned->phone);
        free(records[i].anoned->postal_code);
        free(records[i].anoned->disease);
        free(records[i].anoned);
    }
    free(records);
    // there is one allocation unaccounted for
    // valgrind says it is from fopen somewhere
    // but i can't find it
}
