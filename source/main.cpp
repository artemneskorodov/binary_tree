#include <windows.h>

#include "binary_tree.h"

int main(void) {
SetConsoleCP(1251); //��������� ������� �������� win-cp 1251 � ����� �����
SetConsoleOutputCP(1251);
    char text[256] = {};
    printf("��� ��� ���:");
    scanf("%s", text);

    printf("%s\n", text);
    binary_tree_t tree = {};
    printf("%d\n", binary_tree_init(&tree, 32));
    printf("%d\n", binary_tree_insert(&tree, 10));
    printf("%d\n", binary_tree_dump(&tree, NULL));

    printf("%d\n", binary_tree_insert(&tree, 10));
    printf("%d\n", binary_tree_insert(&tree, 10));
    printf("%d\n", binary_tree_insert(&tree, 40));
    printf("%d\n", binary_tree_insert(&tree, 10));
    printf("%d\n", binary_tree_insert(&tree, 50));
    printf("%d\n", binary_tree_insert(&tree, 20));
    printf("%d\n", binary_tree_insert(&tree, 40));
    printf("%d\n", binary_tree_insert(&tree, 20));
    printf("%d\n", binary_tree_insert(&tree, 60));
    printf("%d\n", binary_tree_insert(&tree, 30));


    printf("%d\n", binary_tree_insert(&tree, 20));

    binary_tree_dump(&tree, tree.data + 4);
    printf("dump");

    binary_tree_print_inorder(&tree);
    fclose(tree.general_dump);
}
