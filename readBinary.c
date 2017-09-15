struct MemoryRegion
{
    void *startAddr;
    void *endAddr;
    int isReadable;
    int isWritable;
    int isExecutable;
};

int main(int argc, char  *argv[]) {

    int *ifp2;
    ifp2 = open("myckpt", 'rb');
    if(ifp2 == -1) {
        printf("%s\n", "Check pointing image not found");
        exit(1);
    }
    else {
        printf("%s\n", "Reading back");
        int c = 0;
        char a[sizeof(struct MemoryRegion)] ;
        c = read(ifp2, a, sizeof(struct MemoryRegion));
        printf("%s\n", "printing");
        printf("%s\n", a);
    }
    close(ifp2);
}
