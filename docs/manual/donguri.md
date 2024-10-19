---
title: 5ch.net どんぐりシステムの利用方法
layout: default
---
<!-- SPDX-License-Identifier: FSFAP OR GPL-2.0-or-later -->

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [どんぐりシステムの概要](#summary)
- [自動ログイン (警備員○)](#auto-login)
- [Gmailのメールアドレスによるログイン (警備員●)](#gmail-login)
- [JDimでは未対応の機能](#not-supported)
- [参考文献](#references)


<a name="summary"></a>
### どんぐりシステムの概要

2024年3月下旬頃から、どんぐりシステムというユーザー識別の仕組みが5ch.netに導入されました。

どんぐりシステムには2種類のアカウントがあります。
- 5ch.netの有料サービスUPLIFTの購読者がログインできる**ハンター**
- UPLIFTの有無に関係なくログインできる**警備員**

警備員でログインするには、次の2つの方法があります。
- **自動ログイン (警備員○)**:  
  ブラウザの設定や特性（IPアドレス、ユーザーエージェントなど）に紐付けられた警備員○アカウントで自動的にログインします。
- **Gmailのメールアドレスによるログイン (警備員●)**:  
  Gmailのメールアドレスで紐付けられた警備員●アカウントでログインします。
  この方法を使うと、複数のブラウザで同じアカウントを使うことができます。

2024年7月時点の最新版JDimでは、どんぐりシステムに警備員アカウントでログインしレスを書き込むことが可能です。

各種エラーメッセージと対応方法や、このマニュアルに載っていない機能の
詳細については[参考文献](#references)のリンクを参照してください。


<a name="auto-login"></a>
### 自動ログイン (警備員○)

1. 警備員●にログインしている場合はログアウトします(ログアウト方法は下記参照)。
2. 5ch.netのどんぐりシステムが導入されている板やスレに書き込むと警備員○アカウントにログインします。
   (自動ログインが可能な板については[参考文献](#references)の「どんぐりシステムが導入されている板一覧」を参照)

> [!NOTE]
> - 自動ログインの有効期限は約24時間で、有効期限が切れた後にレスを書き込むとエラーが発生します。
> - IPアドレスやユーザーエージェントが変化した場合も、ログインが無効になり書き込みエラーが発生することがあります。
> - 書き込みエラーが発生した場合は、再度書き込むことで、自動的に新しい警備員○アカウントにログインできます。


<a name="gmail-login"></a>
### Gmailのメールアドレスによるログイン (警備員●)

<small>v0.11.0-20240616から追加</small>

JDimでは、Gmailのメールアドレスに登録した警備員●アカウントでログインできます。

#### 初期設定
1. メニューバーから`設定`→`ネットワーク`→`パスワード`を開き`どんぐりシステム`タブを選択します。
2. 警備員●アカウント登録済みのGmailのメールアドレスとパスワードを設定し、OKを押してダイアログを閉じます。

#### ログイン
メニューバーから`ファイル`→`どんぐりシステムにGmail警備員●でログイン`を選ぶと警備員●アカウントにログインします。
ログイン後は、警備員●アカウントでレスを書き込むことができます。

ログインした状態でJDimを終了すると、次回起動時に警備員●アカウントにログインします。

#### ログアウト
ログアウトするには、メニューバーから`ファイル`→`どんぐりシステムにGmail警備員●でログイン`を再度選択してください。

#### ログイン状態の確認
メインウインドウのタイトルバーに`[ 警備員● ]`が表示されているときは、警備員●アカウントにログインしています。

#### アカウントの確認
ログインしたアカウントを確認するには、メニューバーから`設定`→`ネットワーク`→`パスワード`を開き、
`どんぐりシステム`のタブを選択すると警備員●の名前とID、HTTPクッキー(acorn)を見ることができます。

> [!NOTE]
> `パスワード`のダイアログにメールアドレスとパスワードを設定した後ログインしないでJDimを終了したときは、
> ダイアログで設定するところからやり直しになります。
> 
> ログインしている状態でHTTPクッキーを削除したり、有効期限が切れたりするとログインが無効になります。
> 無効の状態でレスを書き込むとエラーが発生するので、ログアウトしてから再度ログインしてください。
> 再ログインせずに続けて書き込むと、ログイン状態の表示はそのままで[自動ログイン](#auto-login)に変わってしまうためご注意ください。


<a name="not-supported"></a>
### JDimでは未対応の機能

#### ハンターアカウントでのログイン
JDimではハンターログインに必要なUPLIFTのログインに対応していません。
Webブラウザや他の専ブラでUPLIFTとハンターにログインして書き込みなどを行ってください。

#### Gmailのメールアドレスに警備員●アカウントを登録

JDimでは、Gmailのメールアドレスを使った警備員●アカウントの登録には対応していません。

Webブラウザを使った登録手順は以下の通りです。
1. 5ch.netのどんぐりシステムが導入されている板やスレに書き込むと、自動的に警備員○アカウントでログインします。
2. その状態でどんぐりシステムの[トップページ][donguri-top]を開くと、アカウント情報が見られるどんぐりベースが表示されます。
3. どんぐりベースにある「警備員登録サービス」を開きGmailのメールアドレスとパスワードを入力して登録ボタンを押します。
4. どんぐりシステムのログインページから警備員●アカウントにログインします。

> [!NOTE]
> 登録に使用した警備員○アカウントはそのまま残っており、
> 警備員●アカウントの初期設定は警備員○アカウントの自動ログインが有効になっています。
> 特にIPアドレスを他者と共有する環境で登録した場合はご注意ください。

#### 警備員のスキル修行や武器防具の装備

JDimでは警備員のスキル修行や武器防具の装備には対応していません。
Webブラウザで警備員アカウントにログインし、どんぐりベースからスキル修行や武器防具の装備を行ってください。


<a name="references"></a>
### 参考文献

URL | 説明
--- | ---
<https://donguri.5ch.net>  | どんぐりシステムのトップページ (ログインページとどんぐりベース)
<https://donguri.5ch.net/faq>  | 公式FAQページ
<https://kes.5ch.net/donguri/> | 5ch.net どんぐり板 (どんぐりシステムの話題を扱う)
<http://dongurirank.starfree.jp/acorn.html> | 有志まとめ: どんぐりシステムが導入されている板一覧

[donguri-top]: https://donguri.5ch.net