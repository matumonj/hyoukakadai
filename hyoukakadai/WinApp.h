#pragma once
#include<Windows.h>
class WinApp
{
private:
	
	WNDCLASSEX w{};
	HWND hwnd = nullptr;
public:
	//window�����E�c��
	const int window_width = 900;
	const int window_height = 720;
	RECT wrc;
	
	//�Q�b�^�[ 
	//����hwnd�Ƃ��X���b�v�`�F�[���쐬�Ƃ��Ŏg������
	//dxgifactory�͔ėp�@�\�̂Ƃ���
	//instanceha�͓��͏��������Ŏg��
	HWND GetHwnd() { return hwnd; }
	HINSTANCE GetInstance() { return w.hInstance; }

	//�����o�֐�
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void Createwindow();
	bool Processmassage();
	void Deletewindow();
};

