#include "stdafx.h"

DECLARE_COMPONENT_VERSION(
    "R128 \xE3\x83\xAA\xE3\x82\xA2\xE3\x83\xAB\xE3\x82\xBF\xE3\x82\xA4\xE3\x83\xA0\xE9\x9F\xB3\xE9\x87\x8F\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6\xE3\x83\xBC",
    "1.4.1",
    "foobar2000用のR128ベース・リアルタイム音量ノーマライザーです。\n"
    "\n"
    "Author: Maximum\n"
    "Copyright (c) 2026 Maximum\n"
    "License: MIT License\n"
    "\n"
    "v1.4.1では、保存済み設定から現在のプリセットを自動判定し、\n"
    "設定画面を閉じた後やfoobar2000再起動後にも表示します。\n"
    "手動調整した設定はカスタム設定として表示されます。\n"
    "\n"
    "音声処理、測定計算、プリセット値、設定保存形式、\n"
    "コンポーネントGUIDはv1.4.0と同一です。\n"
    "\n"
    "確認済み表示環境：1920x1080、Windows表示倍率150%。\n"
    "\n"
    "Conceptually based in part on the prior work\n"
    "EBU R128 Normalizer by mudlord.\n"
    "This implementation was independently written.\n"
    "\n"
    "See license.txt and THIRD-PARTY-NOTICES.txt for full notices."
);

VALIDATE_COMPONENT_FILENAME("foo_r128_normalizer.dll");
