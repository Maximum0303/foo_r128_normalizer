#include "stdafx.h"

DECLARE_COMPONENT_VERSION(
    "R128 \xE3\x83\xAA\xE3\x82\xA2\xE3\x83\xAB\xE3\x82\xBF\xE3\x82\xA4\xE3\x83\xA0\xE9\x9F\xB3\xE9\x87\x8F\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6\xE3\x83\xBC",
    "1.4.0",
    "foobar2000用のR128ベース・リアルタイム音量ノーマライザーです。\n"
    "\n"
    "Author: Maximum\n"
    "Copyright (c) 2026 Maximum\n"
    "License: MIT License\n"
    "\n"
    "v1.4.0は最初の安定公開版です。\n"
    "v1.3.10で確認した3タブ設定画面、高DPI対応、\n"
    "異常値保護、診断、音量一致A/B比較を正式版として固定しました。\n"
    "\n"
    "確認済み表示環境：1920x1080、Windows表示倍率150%。\n"
    "その他の解像度・表示倍率は環境により表示が異なる場合があります。\n"
    "\n"
    "音声処理、測定計算、プリセット値、設定保存形式、\n"
    "コンポーネントGUIDはv1.3.10と同一です。\n"
    "\n"
    "Conceptually based in part on the prior work\n"
    "EBU R128 Normalizer by mudlord.\n"
    "This implementation was independently written.\n"
    "\n"
    "See license.txt and THIRD-PARTY-NOTICES.txt for full notices."
);

VALIDATE_COMPONENT_FILENAME("foo_r128_normalizer.dll");
