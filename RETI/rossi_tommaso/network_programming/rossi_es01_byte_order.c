#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    int host_num[5] = {1000, 12890, 54163, 34012, 62984};
    int net_num[5] = {19875, 5499, 59821, 43120, 26489};

    printf("\nHost to network short: \n\n");

    for (int i = 0; i < 5; i++) {
        printf("host %d: net %x\n", host_num[i], htons(host_num[i]));
    }

    printf("\nNetwork to host short: \n\n");

    for (int i = 0; i < 5; i++) {
        printf("net %d: host %x\n", net_num[i], ntohs(net_num[i]));
    }


    return 0;
}
