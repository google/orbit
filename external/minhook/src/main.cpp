#include <iostream>
#include "../include/MinHook.h"
#include "../src/orbitasm.h"

using namespace std;

int main()
{
    cout << "OrbitProlog size: " << sizeof( OrbitProlog ) << endl;
    cout << "OrbitEpilog size: " << sizeof( OrbitEpilog ) << endl;

    MH_Test();

    system("pause");
    return 0;
}
