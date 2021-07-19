#include <Common/Common.h>

class Test {
public:
	Test(int v) {
	}
	~Test() {
		std::cout << "Dispose\n";
	}
};

int main() {
	vengine_init_malloc();
}