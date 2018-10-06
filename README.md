
# 導入方法

> 引用は開発版(alpha)の注意書き

## 事前準備

　一度だけやればいい。

### Redhat系
`  dnf install gtkmm24-devel gnutls-devel libSM-devel libtool automake git `

> gtk3版をビルドする場合は `gtkmm24-devel` の代わりに `gtkmm30-devel` をインストールする。

### Debin系
`   sudo apt-get build-dep jd`

　開発環境が入っていない場合は、

`  sudo apt-get install build-essential automake autoconf-archive git`

### Ubuntu 18.04
　開発環境が入っていない場合は、

`    sudo apt-get install build-essential automake autoconf-archive git libtool`

　必要なライブラリを入れる。(抜けがあるかも)

`    sudo apt-get install libgtkmm-2.4-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev libgcrypt20-dev`

> gtk3版をビルドする場合は `libgtkmm-2.4-dev` の代わりに `libgtkmm-3.0-dev` をインストールする。

## インストール

```
    git clone -b test --depth 1 https://github.com/yama-natuki/JD.git jd  
    cd jd  
    autoreconf -i  
    ./configure  
    make
```

　実行するには直接 src/jd を起動するか手動で /usr/bin あたりに src/jd を cp する。

> **デフォルトではgtk2版になる。** gtk3版をビルドするには ./configure にオプション `--with-gtkmm3` を追加する。
> ビルド/インストールの方法は [INSTALL](./INSTALL) にも書いてある。
> ```sh
> git clone -b gtk3-idea-r2 --depth 1 https://github.com/ma8ma/JD.git jd
> cd jd
> autoreconf -i
> ./configure --with-gtkmm3
> make
> ```

### Arch Linux
> gtk3版のビルドファイルはAURで公開されている。(Thanks to @naniwaKun.)  
> https://aur.archlinux.org/packages/jd-gtk3/
>
> AUR Helper [yay](https://github.com/Jguer/yay) でインストール
> ```sh
> yay -S jd-gtk3
> ```

# 参考
　詳しいインストールの方法は [本家のwiki](https://ja.osdn.net/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95) を参照。


## Tips

### ●buildの高速化
　make するときに -j job数(並列処理の数) を指定すれば高速にコンパイルできます。  
使用するCPUのコア数と相談して決めてください。

###  ●to_string’ is not a member of ‘std’なエラーが出た場合
　このエラーが出た場合は、 automake のマクロが入っていないか、
gcc のバージョンが古い可能性があります。

automakeのマクロはubuntuでは autoconf-archive というパッケージ名です。

またマクロを入れなくても、

`   make CXXFLAGS+="-std=c++11" `

でも同様の効果があります。

　もしこれで駄目な場合はgccのversionが古すぎるので、
gccのバージョンアップをするか、ディストリをバージョンアップしてください。

### ●configureチェック中に `AX_CXX_COMPILE_STDCXX_11(noext, mandatory)` に関連したエラーがでた場合
ubuntuでは `autoconf-archive` をインストールして `autoreconf -i` からやり直してみてください。
パッケージが見つからないまたはエラーが消えない場合は以下の手順を試してみてください。

1. `configure.ac` の `AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])` の行を削除する。
2. `autoreconf -i` で `configure` を作りconfigureチェックをやり直す。
3. `make CXXFLAGS+="-std=c++11"` でビルドする。


# GTK2とGTK3の違い

> * 一部のルック・アンド・フィールが変更されている。
>   (タブの並べ替えとドラッグ・アンド・ドロップ)
>
> * GTK3ではタブの上でマウスホイールするとページが切り替わる機能は削除された。
>
> * 板一覧やスレ一覧でマウスホイールによるスクロールが動作しないことがある。
>   環境変数 `GDK_CORE_DEVICE_EVENTS=1` を設定してjdを起動するとマウスホイール機能が使える。
