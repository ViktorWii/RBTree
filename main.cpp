////////////////////////////////////////////////////////////////////////////////
// Module Name:  main.cpp
// Authors:      Sergey Shershakov
// Version:      0.1.0
// Date:         01.05.2017
//
// This is a part of the course "Algorithms and Data Structures" 
// provided by  the School of Software Engineering of the Faculty 
// of Computer Science at the Higher School of Economics.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "rbtree.h"


using namespace std;


void simplestTest()
{
    using namespace xi;

    // ������ ������� ������ ������
    RBTree<int> tree1;
    tree1.insert(5);
    tree1.insert(7);
    tree1.insert(9);
    tree1.insert(8);
    tree1.insert(2);
    cout << tree1.find(9)->getKey();
    tree1.remove(7);
    tree1.remove(5);
    tree1.remove(2);
    tree1.remove(9);
    tree1.remove(8);
   cout << "hey";

}


int main()
{
    cout << "Hello, World!" << endl;

    simplestTest();

    return 0;
}