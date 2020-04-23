#include <iostream>
#include <memory>
#include "sptr.h"
int main()
{
	SharedPtr<int> sptr(new int);
	//sptr.~SharedPtr();
	WeakPtr<int> wptr(sptr);
	SharedPtr<int> sptr2(wptr.Lock());
	return 0;
}