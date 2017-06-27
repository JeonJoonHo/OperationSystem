#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>
#include <errno.h>

#define FRAME_SIZE 256
#define FRAME_NUMBER 128
#define ADDRESS_MASK 0xFFFF
#define OFFSET_MASK 0xFF
#define TLB_SIZE 16
#define PAGE_SIZE 256

#define BUFFER 10
#define readBytes 256

int logical[PAGE_SIZE] = {0, };
int physical[PAGE_SIZE];

//PAGE, FRAME
int pageNumber[PAGE_SIZE] = {0, };
int pageFrame[PAGE_SIZE] = {0, };
int pageNumbers = 0;
int frameNumbers = 0;

//TLB
int TLBpageNumber[PAGE_SIZE];
int TLBframeNumber[PAGE_SIZE];
int TLBhits = 0;
int TLBEntries = 0;

//LRU
int LRUpageNumber[PAGE_SIZE];
int LRUframeNumber[PAGE_SIZE];
int LRUcounter[PAGE_SIZE];
int LRUhits = 0;

//FIFO
int FIFOpageNumber[PAGE_SIZE];
int FIFOframeNumber[PAGE_SIZE];
int FIFOcounter = 0;
int FIFOhits = 0;

//physicalMemory
int phyMemory[PAGE_SIZE][PAGE_SIZE];

FILE *addressFile;
FILE *addressFile1;
FILE *backingStore;
FILE *physicalAddress;
FILE *physicalMemory;
FILE *frameTa;

int logicalAdress;
int pageFaults = 0;
char address[BUFFER];

// buffer
signed char buffer[PAGE_SIZE];

void getPage(int logical_address);
void TLBtable(int pages, int frames);
void LRUAlgorithm(int pageNumber, int frames, int logical_address);
void FIFOAlgorithm(int pageNumber, int frames, int logical_address);
void backStore(int pages);

int replacementAlgorithmChange = 0;

int main(int argc, char* argv[]) {
    if(argc - 1 != 1) {
        fputs("error, INPUT FILE => addresses.txt BACKING_STORE.bin", stderr);
        exit(0);
    }
    
    if( NULL == (addressFile = fopen( argv[1], "r") ) )
    { // then fopen failed
        fprintf( stderr, "fopen for input file: %s failed due to %s\n", argv[0], strerror(errno) );
        exit( EXIT_FAILURE );
    }
    if( NULL == (addressFile1 = fopen( argv[1], "r") ) )
    { // then fopen failed
        fprintf( stderr, "fopen for input file: %s failed due to %s\n", argv[0], strerror(errno) );
        exit( EXIT_FAILURE );
    }
    
    int addressTranslatedNumber = 0;
    int i, j;
    for(j = 0; j < 2; j++) {
        for(i = 0; i < PAGE_SIZE; i++) {
            pageNumbers = 0;
            frameNumbers = 0;
            pageNumber[i] = 0;
            pageFrame[i] = 0;
            TLBpageNumber[i] = 0;
            TLBframeNumber[i] = 0;
            LRUpageNumber[i] = 0;
            LRUframeNumber[i] = 0;
            LRUcounter[i] = 0;
            FIFOpageNumber[i] = 0;
            FIFOframeNumber[i] = 0;
            FIFOcounter = 0;
            logical[i] = 0;
            physical[i] = 0;
            addressTranslatedNumber = 0;
            pageFaults = 0;
            TLBhits = 0;
        }
        
        int vai = 0;
        if(replacementAlgorithmChange == 0) {
            while (fgets(address, BUFFER, addressFile1) != NULL) {
                logicalAdress = atoi(address);
                
                getPage(logicalAdress);
                addressTranslatedNumber++;
            }
            
            for (i = 0; i < frameNumbers; i++) {
                if( NULL == (frameTa = fopen("Frame_table(LRU).txt", "a") ) )
                { // then fopen failed
                    fprintf( stderr, "fopen for input file: %s failed due to %s\n", "Frame_table(LRU).txt", strerror(errno) );
                    exit( EXIT_FAILURE );
                }
                
                if (pageNumber[i] != 0) vai = 1;
                
                fprintf(frameTa, "%d %d %d\n", pageFrame[i], vai, logical[i]);
                
                fclose(frameTa);
                
                if( NULL == (physicalMemory = fopen("Physical Memory(LRU).bin", "a") ) )
                { // then fopen failed
                    fprintf( stderr, "fopen for input file: %s failed due to %s\n", "Frame_table(LRU).txt", strerror(errno) );
                    exit( EXIT_FAILURE );
                }
                
                fwrite(phyMemory[i], sizeof(unsigned char), readBytes, physicalMemory);
                
                fclose(physicalMemory);
            }
            
            printf("TLB(LRU) hit ratio : %d hits out of %d\n", TLBhits, addressTranslatedNumber);
            printf("LRU hit ratio : %d hits out of %d\n", LRUhits, addressTranslatedNumber);
            replacementAlgorithmChange++;
        }
        else if (replacementAlgorithmChange == 1){
            while (fgets(address, BUFFER, addressFile) != NULL) {
                logicalAdress = atoi(address);
                
                getPage(logicalAdress);
                addressTranslatedNumber++;
            }
            
            for(i = 0; i < frameNumbers; i++) {
                if( NULL == (frameTa = fopen("Frame_table(FIFO).txt", "a") ) )
                { // then fopen failed
                    fprintf( stderr, "fopen for input file: %s failed due to %s\n", "Frame_table(FIFO).txt", strerror(errno) );
                    exit( EXIT_FAILURE );
                }
                
                if(pageNumber[i] != 0) vai = 1;
                
                fprintf(frameTa, "%d %d %d\n", pageFrame[i], vai, logical[i]);
                
                fclose(frameTa);
                
                if( NULL == (physicalMemory = fopen("Physical Memory(FIFO).bin", "a") ) )
                { // then fopen failed
                    fprintf( stderr, "fopen for input file: %s failed due to %s\n", "Frame_table(FIFO).txt", strerror(errno) );
                    exit( EXIT_FAILURE );
                }
                
                fwrite(phyMemory[i], sizeof(unsigned char), readBytes, physicalMemory);
                
                fclose(physicalMemory);
            }
            
            printf("TLB(FIFO) hit ratio : %d hits out of %d\n", TLBhits, addressTranslatedNumber);
            printf("FIFO hit ratio : %d hits out of %d\n", FIFOhits, addressTranslatedNumber);
        }
    }
    
    fclose(addressFile);
    fclose(addressFile1);
    return 0;
}

void backStore(int pages){
    if( NULL == (backingStore = fopen( "BACKING_STORE.bin", "rb") ) )
    { // then fopen failed
        fprintf( stderr, "fopen for input file: %s failed due to %s\n", "BACKING_STORE.bin", strerror(errno) );
        exit( EXIT_FAILURE );
    }
    
    if( 0 != fseek(backingStore, pages * readBytes, SEEK_SET) ){
        fprintf(stderr, "Error seeking in backing store\n");
        exit( EXIT_FAILURE );
    }
    
    if (fread(buffer, sizeof(signed char), readBytes, backingStore) == 0) {
        fprintf(stderr, "Error reading from backing store\n");
        exit( EXIT_FAILURE );
    }
    
    int i;
    for (i = 0; i < readBytes; i++) {
        phyMemory[frameNumbers][i] = buffer[i];
    }
    
    fclose(backingStore);
}

void getPage(int logical_address) {
    if(replacementAlgorithmChange == 0) {
        if( NULL == (physicalAddress = fopen("physical(LRU).txt", "a") ) )
        { // then fopen failed
            fprintf( stderr, "fopen for input file: %s failed due to %s\n", "physical(LRU).txt", strerror(errno) );
            exit( EXIT_FAILURE );
        }
    }
    else if(replacementAlgorithmChange == 1){
        if( NULL == (physicalAddress = fopen("physical(FIFO).txt", "a") ) )
        { // then fopen failed
            fprintf( stderr, "fopen for input file: %s failed due to %s\n", "physical(FIFO).txt", strerror(errno) );
            exit( EXIT_FAILURE );
        }
    }
    
    int page = ((logical_address & ADDRESS_MASK) >> 8);
    int offset = (logical_address & OFFSET_MASK);
    
    int frame = -1;
    
    int i, j;
    for(i = 0; i < TLB_SIZE; i++) {
        if(TLBpageNumber[i] == page){
            frame = TLBframeNumber[i];
            logical[i] = logical_address;
            TLBhits++;
            
            if(replacementAlgorithmChange == 0) LRUhits++;
            else if(replacementAlgorithmChange == 1) FIFOhits++;
            
            for (j = 0; j < frameNumbers; j++) {
                if (page == LRUpageNumber[j]) {
                    LRUcounter[j] = 0;
                }
                LRUcounter[j]++;
            }
        }
    }
    
    if(frame == -1) {
        if (frameNumbers < FRAME_NUMBER) {
            if (frame == -1) {
                for (i = 0; i < frameNumbers; i++) {
                    if (pageNumber[i] == page) {
                        frame = pageFrame[i];
                        logical[i] = logical_address;
                        
                        for(j = 0; j < frameNumbers; j++) {
                            if(replacementAlgorithmChange == 0) {
                                if (page == LRUpageNumber[j]) {
                                    LRUhits++;
                                    LRUcounter[j] = 0;
                                }
                            }
                            if(replacementAlgorithmChange == 1) {
                                if (page == FIFOpageNumber[j]) {
                                    FIFOhits++;
                                }
                            }
                        }
                    }
                }
                
                if (frame == -1) {
                    logical[frameNumbers] = logical_address;
                    pageNumber[frameNumbers] = page;
                    pageFrame[frameNumbers] = frameNumbers;
                    LRUpageNumber[frameNumbers] = page;
                    LRUframeNumber[frameNumbers] = frameNumbers;
                    FIFOpageNumber[frameNumbers] = page;
                    FIFOframeNumber[frameNumbers] = frameNumbers;
                    
                    frameNumbers++;
                    pageFaults++;
                }
                
                for (i = 0; i < frameNumbers; i++) {
                    LRUcounter[i]++;
                }
            }
        }
        else {
            if(replacementAlgorithmChange == 0) {
                LRUAlgorithm(page, frame, logical_address);
            }
            else {
                FIFOAlgorithm(page, frame, logical_address);
            }
        }
    }
    
    for(i = 0; i < frameNumbers; i++) {
        if(page == pageNumber[i]) {
            frame = pageFrame[i];
        }
    }
    
    pageNumbers++;
    
    TLBtable(page, frame);
    backStore(page);
    fprintf(physicalAddress, "Virtual address: %d Physical address: %d\n", logical_address, (frame << 8) | offset);
    
    fclose(physicalAddress);
}

void LRUAlgorithm(int pages, int frames, int logical_address) {
    int i, j, r;
    
    int tmp[PAGE_SIZE];
    
    for(i = 0; i < frameNumbers; i++) {
        if(pageNumber[i] == pages) {
            frames = pageFrame[i];
            logical[i] = logical_address;
            
            for(j = 0; j < frameNumbers; j++) {
                if(pages == LRUpageNumber[j]) {
                    LRUhits++;
                    LRUcounter[j] = 0;
                }
            }
        }
    }
    
    if(frames == -1) {
        for (r = 0; r < frameNumbers; r++) {
            tmp[r] = LRUcounter[r];
        }
        
        for (r = 0; r < frameNumbers - 1; r++) {
            for (j = 0; j < frameNumbers - 1 - r; j++) {
                if (tmp[j] < tmp[j + 1]) {
                    int t = tmp[j];
                    tmp[j] = tmp[j + 1];
                    tmp[j + 1] = t;
                }
            }
        }
        for (r = 0; r < frameNumbers; r++) {
            if (LRUcounter[r] == tmp[0]) {
                LRUpageNumber[r] = pages;
                logical[r] = logical_address;
                LRUcounter[r] = 0;
            }
        }
        
        pageFaults++;
    }
    for(i = 0; i < frameNumbers; i++) {
        LRUcounter[i]++;
        pageNumber[i] = LRUpageNumber[i];
        pageFrame[i] = LRUframeNumber[i];
    }
}

void FIFOAlgorithm(int pages, int frames, int logical_address) {
    int i;
    
    for(i = 0; i < frameNumbers; i++) {
        if(pages == pageNumber[i]) {
            frames = pageFrame[i];
            logical[i] = logical_address;
            
            if(pages == FIFOpageNumber[i]) {
                FIFOhits++;
            }
        }
    }
    
    if(frames == -1) {
        FIFOpageNumber[FIFOcounter] = pages;
        logical[FIFOcounter] = logical_address;
        
        FIFOcounter = (FIFOcounter + 1) % frameNumbers;
        
        pageFaults++;
    }
    
    for(i = 0; i < frameNumbers; i++) {
        pageNumber[i] = FIFOpageNumber[i];
        pageFrame[i] = FIFOframeNumber[i];
    }
}

void TLBtable(int pages, int frames){
    int i, j;
    for(i = 0; i < TLBEntries; i++){
        if(TLBpageNumber[i] == pages){
            break;
        }
    }
    
    if(i == TLBEntries){
        if(TLBEntries < TLB_SIZE){
            TLBpageNumber[TLBEntries] = pages;
            TLBframeNumber[TLBEntries] = frames;
        }
        else{
            for(j = 0; j < TLB_SIZE - 1; j++){
                TLBpageNumber[j] = TLBpageNumber[j + 1];
                TLBframeNumber[j] = TLBframeNumber[j + 1];
            }
            TLBpageNumber[TLBEntries-1] = pages;
            TLBframeNumber[TLBEntries-1] = frames;
        }
    }
    else{
        for(j = i; j < TLBEntries - 1; j++){
            TLBpageNumber[j] = TLBpageNumber[j + 1];
            TLBframeNumber[j] = TLBframeNumber[j + 1];
        }
        if(TLBEntries < TLB_SIZE){
            TLBpageNumber[TLBEntries] = pages;
            TLBframeNumber[TLBEntries] = frames;
        }
        else{
            TLBpageNumber[TLBEntries-1] = pages;
            TLBframeNumber[TLBEntries-1] = frames;
        }
    }
    
    if(TLBEntries < TLB_SIZE){
        TLBEntries++;
    }
}

