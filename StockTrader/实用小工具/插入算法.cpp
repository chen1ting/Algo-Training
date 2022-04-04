#include <iostream>
using namespace std;
const int Count = 10;
struct SomeStruct
{
	long long OrderIndex;
	//long long BizIndex;
};

void move(int start, int end, SomeStruct* arr) //把该位置之后的所有数后移
{
	for (int i = end; i >= start; i--)
		*(arr + i + 1) = *(arr + i);
}

void print(SomeStruct* arr)
{
	SomeStruct* ptr = arr;
	for (int i = 0; i < Count; i++)
		cout << (ptr++)->OrderIndex << "\t";
	cout << endl;
}

int main()
{
	SomeStruct* buffer = (SomeStruct*)malloc(sizeof(SomeStruct) * Count);
	SomeStruct* ini = buffer;
	//initialize struct array
	for (int i=0;i<8;i++)
		(ini++)->OrderIndex = 10 - i;
	(ini ++)->OrderIndex = 10;
	(ini ++)->OrderIndex = 9;
	print(buffer);

	SomeStruct* cur = buffer;
	SomeStruct* next = buffer + 1;

	int size = Count;
	for (int i = 0; i < size; i++)
	{	
		bool Repeated = false;
		if (cur->OrderIndex < next->OrderIndex)
		{ 
			cur++;
			next++;
			continue;
		}
		else
		{
			long long wrongNum = next->OrderIndex;
			SomeStruct flag = *next;
			int pos = 0;
			for (int j = i; j >= 0; j--)
			{
				if ((buffer + j)->OrderIndex == wrongNum)
				{
					Repeated = true; 
					break;
				}
				else if ((buffer + j)->OrderIndex < wrongNum)
				{
					pos = j; 
					break;
				}	
			}
			if (!Repeated)
			{	
				int j = i;
				for (; j >= pos; j--)
					*(buffer + j + 1) = *(buffer + j);
				*(buffer + j + 1) = flag;
				cur++; next++;
				continue;
			}
			else
			{
				size--;
				for (int j = i; j < size; j++)
					*(buffer + j) = *(buffer + j + 1);
				continue;
			}
		}

	}
	cout << size<<endl;
	print(buffer);
}
