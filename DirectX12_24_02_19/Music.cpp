#include "Music.h"

BGMObject::BGMObject()
{
    // 1. まず音声ファイルを開く（"my_sound" という名前を付ける）
    mciSendString(TEXT("open \"蒲鉾さちこ-ドラマティック・シティ.wav\" type waveaudio alias my_sound"), NULL, 0, NULL);
}

BGMObject::~BGMObject()
{
    // 4. 使い終わったら閉じる（このコードでは到達しませんが、終了時に必要）
     mciSendString(TEXT("close my_sound"), NULL, 0, NULL);
}

void BGMObject::initialize() {
}
void BGMObject::Update() {


    //while (true) {
        // 2. 現在の状態を取得
        mciSendString(TEXT("status my_sound mode"), status, 128, NULL);

        // 3. 停止中（stopped）なら再生する
        if (wcscmp(status, TEXT("stopped")) == 0 || wcscmp(status, TEXT("")) == 0) {
            // 最初から再生
            mciSendString(TEXT("play my_sound from 0"), NULL, 0, NULL);
        }

        // ループが速すぎてCPUに負荷をかけないよう、少し待つ
        //Sleep(100);
    //}

}