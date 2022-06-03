#include <windows.h>
#include <ctime>
#include <stdlib.h>
#include <iostream>
using namespace std;

__int16* stack;
HANDLE SemaphoreAdd;
HANDLE SemaphoreRemove;
CRITICAL_SECTION cs;

class MonitorStack {
private:
	int maxSize, currentSize;
public:
	MonitorStack() {
		maxSize = 0;
		currentSize = 0;
	}

	MonitorStack(int nSize) // nSize - размер очереди
	{
		stack = new __int16[nSize];
		maxSize = nSize;
		currentSize = 0;
	};

	void Push(__int16& nElement) // добавить элемент в стек
	{
		WaitForSingleObject(SemaphoreAdd, INFINITE);
		stack[currentSize] = nElement;
		currentSize++;
		ReleaseSemaphore(SemaphoreRemove, 1, NULL);//освобождаем счетчик (хенд семафора, на сколько изменится счетчик, предыдущее значение)
	};

	__int16 Pop() // удалить элемент из стека
	{
		WaitForSingleObject(SemaphoreRemove, INFINITE);
		__int16 elem = stack[currentSize - 1];
		currentSize--;
		ReleaseSemaphore(SemaphoreAdd, 1, NULL);
		return elem;
	};
};

MonitorStack monitor;

DWORD WINAPI consume(LPVOID count) {
	int cnt = (int)count;
	for (int i = 0; i < cnt; i++) {
		unsigned int obj = monitor.Pop();//потребить (удалить из очереди\стека) заданное количество продуктов;
		EnterCriticalSection(&cs);
		cout << "Consumed object " << obj << endl;//при извлечении очередного символа из очереди (стека), выводить на консоль сообщение следующего вида: "\tУпотреблен элемент (продукт) N ", где N – элемент(продукт) который извлечен из очереди\стека
		LeaveCriticalSection(&cs);
		Sleep(500);
	}
	return 0;
}

DWORD WINAPI produce(LPVOID count) {
	int cnt = (int)count;
	for (int i = 0; i < cnt; i++) {
		__int16 obj = (__int16)rand() % 100; // доступ к элементу
		EnterCriticalSection(&cs);
		cout << "Produced object " << obj << endl;//после добавления очередного числа в очередь(стек) выводить на консоль сообщение следующего вида: "Произведен элемент (продукт) N ", где N, записано в очередь\стек
		LeaveCriticalSection(&cs);
		monitor.Push(obj);
		Sleep(500);
	}
	return 0;
}

void main() {
	int consumers, producers, size;
	int* consumed;
	int* produced;

	HANDLE* handles;
	DWORD* consumersID;
	DWORD* producersID;

	cout << "Input number of consumers: " << endl;
	cin >> consumers;
	consumed = new int[consumers];

	cout << "Input number of producers: " << endl;
	cin >> producers;
	produced = new int[producers];

	cout << "Input size of queue: " << endl;
	cin >> size;

	monitor = MonitorStack(size);//создать монитор очереди (стека), размер которой вводится пользователем с клавиатуры;
	SemaphoreAdd = CreateSemaphore(NULL, size, size, NULL);
	SemaphoreRemove = CreateSemaphore(NULL, 0, size, NULL);

	InitializeCriticalSection(&cs);// указатель на переменную критическая секция

	handles = new HANDLE[consumers + producers];
	consumersID = new DWORD[consumers];
	producersID = new DWORD[producers];
	int pos = 0;

	//ввести с клавиатуры количество потоков producer и количество потоков consumer, которые он должен запустить
	for (int i = 0; i < consumers; i++) {
		cout << "Input number of consumed objects " << endl;
		cin >> consumed[i];
	}

	for (int i = 0; i < producers; i++) {
		cout << "Input number of produced objects " << endl;
		cin >> produced[i];
	}
	srand(time(0));
	//создать требуемое количество потоков producer и consumer;
	for (int i = 0; i < producers; i++, pos++)
		handles[i] = CreateThread(NULL, 0, produce, (int*)produced[i], 0, &producersID[i]);

	for (int i = 0; i < consumers; i++, pos++)
		handles[pos] = CreateThread(NULL, 0, consume, (int*)consumed[i], 0, &consumersID[i]);
	WaitForMultipleObjects(consumers + producers, handles, TRUE, INFINITE);

	cout << "Main finished" << endl;
	system("pause");
};


