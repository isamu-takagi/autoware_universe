# autoware_system_mode_decider

## Overview

## Drive Mode Status

| Flags                  | Description                                                      |
| ---------------------- | ---------------------------------------------------------------- |
| background available   | Autowareが車両を制御していない状況で該当モードに遷移できるか。   |
| background continuable | Autowareが車両を制御していない状況で現在のモードを継続できるか。 |
| foreground available   | Autowareが車両を制御している状況で該当モードに遷移できるか。     |
| foreground continuable | Autowareが車両を制御している状況で現在のモードを継続できるか。   |
| ready                  | 該当モードの出力が実際に選択できる状態になっているか。           |
| stable                 | 該当モードの動作が安定し遷移を完了できる状態になっているか。     |

## Autoware Mode Transition (Background)

車両が手動操作を行っている間にAutowareのモードを切り替えます。これは主に走行中モードチェンジを行う前段階として使用されます。

## Autoware Mode Transition (Foreground)

1. 遷移フィルターを有効化する。
2. 遷移フィルターを無効化する。

## Platform mode Transition

無効化の場合は直ちに車両インターフェースへモード変更が要求されます。
有効化の場合は以下の遷移判定と遷移処理を行います。

遷移判定

- 現在の operation mode の foreground available が有効かどうかを調べます。
- 走行中に追加条件を必要する場合は foreground available に含めてください。

遷移処理

1. 遷移フィルターを有効化する。
2. 現在のmodeのreadyを待つ
3. 車両モードを切り替える
4. 現在のmodeのstableを待つ
5. 遷移フィルターを無効化する。
