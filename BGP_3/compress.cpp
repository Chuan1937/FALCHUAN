//
// Created by HP183 on 2024/5/16.
//
#include "istream"
int main() {
    int n;
    scanf("%d", &n);
    int a[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    int cnt = 1;
    for (int i = 1; i < n; i++) {
        if (a[i] != a[i - 1]) {
            cnt++;
        }
    }
    printf("%d\n", cnt);
    return 0;
}