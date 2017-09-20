#include "uio-user.h"

int main(){
	UIO * test = UIO_MAP(1, 1);
	UIO * test2 = UIO_MAP(0, 0);
	printf("Map test 1 successful @ 0x%x\n", test->mapPtr);
	printf("Map test 2 successful @ 0x%x\n", test2->mapPtr);
	UIO_UNMAP(test);
	UIO_UNMAP(test2);
	return 1;
}
