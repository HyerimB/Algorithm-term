#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ������ Ʈ�� ��� ����ü
typedef struct HuffmanNode {
    unsigned char data;          // ���� ������
    int frequency;              // �󵵼�
    struct HuffmanNode* left;   // ���� �ڽ�
    struct HuffmanNode* right;  // ������ �ڽ�
} HuffmanNode;

// ������ �ڵ� ����ü
typedef struct {
    unsigned char data;
    char* code;
} HuffmanCode;

// ���� ����
HWND hInput, hOutput, hCompressBtn, hDecompressBtn, hStatus;
HINSTANCE g_hInstance;
HuffmanNode* huffmanRoot = NULL;
HuffmanCode huffmanCodes[256];
char compressedData[4096];
int compressedSize = 0;

// �� ���� �Լ���
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

// �� ��� ����
HuffmanNode* createNode(unsigned char data, int freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->data = data;
    node->frequency = freq;
    node->left = node->right = NULL;
    return node;
}

// ������ �ڵ� ����
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

// �ؽ�Ʈ ����
void compressText(const char* text) {
    // �󵵼� ���
    int freq[256] = { 0 };
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        freq[(unsigned char)text[i]]++;
    }

    // �� ����
    int heapSize = 0;
    HuffmanNode* minHeap[256];
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            minHeap[heapSize++] = createNode(i, freq[i]);
        }
    }

    // �� ����
    for (int i = (heapSize - 1) / 2; i >= 0; i--) {
        minHeapify(minHeap, i, heapSize);
    }

    // ������ Ʈ�� ����
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

    // �ڵ� ����
    char code[256] = { 0 };
    for (int i = 0; i < 256; i++) {
        huffmanCodes[i].code = NULL;
    }
    generateCodes(huffmanRoot, code, 0, huffmanCodes);

    // ���� ������ ����
    compressedSize = 0;
    for (int i = 0; i < len; i++) {
        strcat(compressedData, huffmanCodes[(unsigned char)text[i]].code);
    }
    compressedSize = strlen(compressedData);
}

// �ؽ�Ʈ ���� ����
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

// ������ ���ν���
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // �Է� �ؽ�Ʈ ���̺�
        CreateWindow("STATIC", "�Է� �ؽ�Ʈ:",
            WS_VISIBLE | WS_CHILD,
            10, 10, 100, 20,
            hwnd, NULL, g_hInstance, NULL);

        // �Է� �ؽ�Ʈ ����
        hInput = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
            ES_AUTOVSCROLL | WS_VSCROLL,
            10, 30, 460, 100,
            hwnd, NULL, g_hInstance, NULL);

        // ���� ��ư
        hCompressBtn = CreateWindow("BUTTON", "����",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            10, 140, 100, 30,
            hwnd, (HMENU)1, g_hInstance, NULL);

        // ���� ���� ��ư
        hDecompressBtn = CreateWindow("BUTTON", "���� ����",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            120, 140, 100, 30,
            hwnd, (HMENU)2, g_hInstance, NULL);

        // ��� ���̺�
        CreateWindow("STATIC", "���:",
            WS_VISIBLE | WS_CHILD,
            10, 180, 100, 20,
            hwnd, NULL, g_hInstance, NULL);

        // ��� �ؽ�Ʈ ����
        hOutput = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
            ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            10, 200, 460, 100,
            hwnd, NULL, g_hInstance, NULL);

        // ���� ǥ����
        hStatus = CreateWindow("STATIC", "�غ�",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            10, 310, 460, 20,
            hwnd, NULL, g_hInstance, NULL);

        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {  // ���� ��ư
            char input[1024] = { 0 };
            GetWindowText(hInput, input, 1024);

            if (strlen(input) == 0) {
                SetWindowText(hStatus, "�Է� �ؽ�Ʈ�� �Է��ϼ���!");
                return 0;
            }

            // ���� ������ �ʱ�ȭ
            memset(compressedData, 0, sizeof(compressedData));
            if (huffmanRoot) {
                // ���� Ʈ�� ���� �ڵ� �߰� �ʿ�
                huffmanRoot = NULL;
            }

            // ���� ����
            compressText(input);

            // ��� ǥ��
            char result[1024];
            sprintf(result, "����� ������ ũ��: %d ��Ʈ\n���� ũ��: %d ����Ʈ\n"
                "�����: %.2f%%\n\n����� ������:\n%s",
                compressedSize,
                strlen(input),
                (1.0 - (float)compressedSize / (strlen(input) * 8)) * 100,
                compressedData);

            SetWindowText(hOutput, result);
            SetWindowText(hStatus, "���� �Ϸ�!");
        }
        else if (LOWORD(wParam) == 2) {  // ���� ���� ��ư
            if (compressedSize == 0) {
                SetWindowText(hStatus, "���� �ؽ�Ʈ�� �����ϼ���!");
                return 0;
            }

            char decompressed[1024] = { 0 };
            decompressText(decompressed);

            SetWindowText(hOutput, decompressed);
            SetWindowText(hStatus, "���� ���� �Ϸ�!");
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

    // ������ Ŭ���� ���
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "HuffmanCompressor";

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "������ Ŭ���� ��� ����!", "����", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // ������ ����
    HWND hwnd = CreateWindow(
        "HuffmanCompressor",
        "Huffman ���� ���α׷�",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "������ ���� ����!", "����", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // �޽��� ����
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}