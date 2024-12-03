#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 허프만 트리 노드 구조체
typedef struct HuffmanNode {
    unsigned char data;          // 문자 데이터
    int frequency;              // 빈도수
    struct HuffmanNode* left;   // 왼쪽 자식
    struct HuffmanNode* right;  // 오른쪽 자식
} HuffmanNode;

// 허프만 코드 구조체
typedef struct {
    unsigned char data;
    char* code;
} HuffmanCode;

// 전역 변수
HWND hInput, hOutput, hCompressBtn, hDecompressBtn, hStatus;
HINSTANCE g_hInstance;
HuffmanNode* huffmanRoot = NULL;
HuffmanCode huffmanCodes[256];
char compressedData[4096];
int compressedSize = 0;

// 힙 관련 함수들
void minHeapify(HuffmanNode** minHeap, int idx, int size) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < size && minHeap[left]->frequency < minHeap[smallest]->frequency)
        smallest = left;

    if (right < size && minHeap[right]->frequency < minHeap[smallest]->frequency)
        smallest = right;

    if (smallest != idx) {
        HuffmanNode* temp = minHeap[idx];
        minHeap[idx] = minHeap[smallest];
        minHeap[smallest] = temp;
        minHeapify(minHeap, smallest, size);
    }
}

// 새 노드 생성
HuffmanNode* createNode(unsigned char data, int freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->data = data;
    node->frequency = freq;
    node->left = node->right = NULL;
    return node;
}

// 허프만 코드 생성
void generateCodes(HuffmanNode* root, char* code, int depth, HuffmanCode* codes) {
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        code[depth] = '\0';
        codes[root->data].data = root->data;
        codes[root->data].code = _strdup(code);
        return;
    }

    code[depth] = '0';
    generateCodes(root->left, code, depth + 1, codes);
    code[depth] = '1';
    generateCodes(root->right, code, depth + 1, codes);
}

// 텍스트 압축
void compressText(const char* text) {
    // 빈도수 계산
    int freq[256] = { 0 };
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        freq[(unsigned char)text[i]]++;
    }

    // 힙 생성
    int heapSize = 0;
    HuffmanNode* minHeap[256];
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            minHeap[heapSize++] = createNode(i, freq[i]);
        }
    }

    // 힙 구성
    for (int i = (heapSize - 1) / 2; i >= 0; i--) {
        minHeapify(minHeap, i, heapSize);
    }

    // 허프만 트리 구축
    while (heapSize > 1) {
        HuffmanNode* left = minHeap[0];
        minHeap[0] = minHeap[--heapSize];
        minHeapify(minHeap, 0, heapSize);

        HuffmanNode* right = minHeap[0];
        minHeap[0] = minHeap[--heapSize];
        minHeapify(minHeap, 0, heapSize);

        HuffmanNode* parent = createNode('$', left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;

        minHeap[heapSize++] = parent;
        minHeapify(minHeap, 0, heapSize);
    }

    huffmanRoot = minHeap[0];

    // 코드 생성
    char code[256] = { 0 };
    for (int i = 0; i < 256; i++) {
        huffmanCodes[i].code = NULL;
    }
    generateCodes(huffmanRoot, code, 0, huffmanCodes);

    // 압축 데이터 생성
    compressedSize = 0;
    for (int i = 0; i < len; i++) {
        strcat(compressedData, huffmanCodes[(unsigned char)text[i]].code);
    }
    compressedSize = strlen(compressedData);
}

// 텍스트 압축 해제
void decompressText(char* result) {
    if (!huffmanRoot || !compressedData[0]) {
        strcpy(result, "");
        return;
    }

    HuffmanNode* current = huffmanRoot;
    int index = 0;
    int resultIndex = 0;

    while (compressedData[index]) {
        if (compressedData[index] == '0')
            current = current->left;
        else
            current = current->right;

        if (current->left == NULL && current->right == NULL) {
            result[resultIndex++] = current->data;
            current = huffmanRoot;
        }
        index++;
    }
    result[resultIndex] = '\0';
}

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // 입력 텍스트 레이블
        CreateWindow("STATIC", "입력 텍스트:",
            WS_VISIBLE | WS_CHILD,
            10, 10, 100, 20,
            hwnd, NULL, g_hInstance, NULL);

        // 입력 텍스트 상자
        hInput = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
            ES_AUTOVSCROLL | WS_VSCROLL,
            10, 30, 460, 100,
            hwnd, NULL, g_hInstance, NULL);

        // 압축 버튼
        hCompressBtn = CreateWindow("BUTTON", "압축",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            10, 140, 100, 30,
            hwnd, (HMENU)1, g_hInstance, NULL);

        // 압축 해제 버튼
        hDecompressBtn = CreateWindow("BUTTON", "압축 해제",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            120, 140, 100, 30,
            hwnd, (HMENU)2, g_hInstance, NULL);

        // 결과 레이블
        CreateWindow("STATIC", "결과:",
            WS_VISIBLE | WS_CHILD,
            10, 180, 100, 20,
            hwnd, NULL, g_hInstance, NULL);

        // 출력 텍스트 상자
        hOutput = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
            ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            10, 200, 460, 100,
            hwnd, NULL, g_hInstance, NULL);

        // 상태 표시줄
        hStatus = CreateWindow("STATIC", "준비",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            10, 310, 460, 20,
            hwnd, NULL, g_hInstance, NULL);

        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {  // 압축 버튼
            char input[1024] = { 0 };
            GetWindowText(hInput, input, 1024);

            if (strlen(input) == 0) {
                SetWindowText(hStatus, "입력 텍스트를 입력하세요!");
                return 0;
            }

            // 기존 데이터 초기화
            memset(compressedData, 0, sizeof(compressedData));
            if (huffmanRoot) {
                // 기존 트리 정리 코드 추가 필요
                huffmanRoot = NULL;
            }

            // 압축 수행
            compressText(input);

            // 결과 표시
            char result[1024];
            sprintf(result, "압축된 데이터 크기: %d 비트\n원본 크기: %d 바이트\n"
                "압축률: %.2f%%\n\n압축된 데이터:\n%s",
                compressedSize,
                strlen(input),
                (1.0 - (float)compressedSize / (strlen(input) * 8)) * 100,
                compressedData);

            SetWindowText(hOutput, result);
            SetWindowText(hStatus, "압축 완료!");
        }
        else if (LOWORD(wParam) == 2) {  // 압축 해제 버튼
            if (compressedSize == 0) {
                SetWindowText(hStatus, "먼저 텍스트를 압축하세요!");
                return 0;
            }

            char decompressed[1024] = { 0 };
            decompressText(decompressed);

            SetWindowText(hOutput, decompressed);
            SetWindowText(hStatus, "압축 해제 완료!");
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;

    // 윈도우 클래스 등록
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "HuffmanCompressor";

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "윈도우 클래스 등록 실패!", "오류", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 윈도우 생성
    HWND hwnd = CreateWindow(
        "HuffmanCompressor",
        "Huffman 압축 프로그램",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "윈도우 생성 실패!", "오류", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 메시지 루프
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}