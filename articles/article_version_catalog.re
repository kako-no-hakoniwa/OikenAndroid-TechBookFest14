= Version Catalogを使ったバージョンを一元管理する

== はじめに

2022年1月からAndroidアプリ開発をはじめたknyackikoです。

現在のおいしい健康では、ライブラリのバージョン管理に@<code>{ext}ブロック（Gradleのextraプロパティ）を使用しています。
しかし、@<code>{ext}ブロックを使用する方法ではAndroid Studioで定義ジャンプや補完が効かないなどの不便な点があります。
加えて、おいしい健康ではライブラリの定期的なバージョン更新を効率よく進められていない問題もありました。
そこで、ライブラリのバージョン管理方法をVersion Catalogへ移行し、プロジェクトの依存関係を自動更新できるツールと連携しようと対応中です。
今回はその中で得られた知見を共有します。

なお、この章で使用する各バージョンは次のとおりです。

 * Android Studio Giraffe 2022.3.1 Beta 1
 ** 一部、過去バージョンとの比較でAndroid Studio Flamingo 2022.2.1 Patch 1を使用
 * Gradle 8.0.1

また、Gradleのビルド構成はGroovyを前提とします。

== Version Catalogとは

Version Catalog（バージョンカタログ）は、ライブラリの依存関係やプラグインを一元管理する方法の1つです。
Gradle 7.0でfeature previewとして導入された機能@<fn>{gradle-7.0}で、Gradle 7.4以降@<fn>{gradle-7.4}からは標準的な機能として使用できます。

//footnote[gradle-7.0][https://docs.gradle.org/7.0/userguide/platforms.html]
//footnote[gradle-7.4][https://docs.gradle.org/7.4/userguide/platforms.html]

=== バージョンを一元管理する理由

ライブラリの依存関係やプラグインの一元管理は、プロジェクトがマルチモジュール構成のときにとくに恩恵を受けることができます。
build.gradleにベタ書きする方法を採用しているマルチモジュール環境の場合、複数のモジュールで使用されているライブラリをアップデートする際に、ライブラリを使用しているモジュールすべてで更新作業をする必要があります。
抜け漏れが発生する可能性が容易に考えられます。

//list[VersionCatalog1][build.gradleにベタ書きする場合][groovy]{
dependencies {
    implementation "androidx.appcompat:appcompat:1.6.1"
}
//}

一方、バージョンを一元管理している場合には、定義をしている1箇所のファイルで更新するだけでOKなのでメンテナンス性が高いといえるでしょう。

//list[VersionCatalog2][Version Catalogで一元管理する場合（定義側）][toml]{
[libraries]
androidx-appcompat = { module = "androidx.appcompat:appcompat", version = "1.6.1" }
//}

//list[VersionCatalog3][Version Catalogで一元管理する場合（使用する側）][groovy]{
dependencies {
    implementation libs.androidx.appcompat
}
//}

== Version Catalogを導入する

=== どこで定義するか

Version Catalogは@<code>{settings.gradle}ファイルまたはTOMLファイルで定義できます。

//list[VersionCatalog4][settings.gradleで定義する][groovy]{
dependencyResolutionManagement {
    versionCatalogs {
        libs {
            version("androidx-core", "1.9.0")
            alias("androidx-core-ktx").to("androidx.core", "core-ktx").versionRef("androidx-core")
        }
    }
}
//}

//list[VersionCatalog5][TOMLファイル（libs.versions.toml）で定義する][toml]{
[versions]
androidx-core = "1.9.0"

[libraries]
androidx-core-ktx = { module = "androidx.core:core-ktx", version.ref = "androidx-core" }
//}

今回は、Android Developersで紹介されているTOMLファイルでの定義@<fn>{definition}を前提とします。

//footnote[definition][https://developer.android.com/studio/build/migrate-to-catalogs?hl=ja]

=== TOMLファイルを作成する

ルートプロジェクトのgradleフォルダ内で@<code>{libs.versions.toml}ファイルを作成するのが、簡単かつAndroid公式でもオススメされている方法です。

//footnote[recommendation][https://developer.android.com/studio/build/migrate-to-catalogs?hl=ja#:~:text=Gradle は、デフォルトで libs.versions.toml ファイルでカタログを検索するため、このデフォルト名を使用することをおすすめします。]

//list[VersionCatalog6][TOMLファイルのデフォルト名と作成場所][bash]{
.
├── app
│   ├── .gitignore
│   ├── build.gradle
│   └── src
├── gradle
│   └── libs.versions.toml
├── build.gradle
├── gradle.properties
├── gradlew
├── gradlew.bat
└── settings.gradle
//}

Gradleはルートプロジェクトのgradleフォルダ内にある@<code>{libs.versions.toml}ファイルをデフォルトでカタログファイルとして認識してくれるようになっています。

==== デフォルトのカタログファイル以外を使用する

デフォルトの@<code>{libs.versions.toml}ファイル以外のファイル名でカタログを追加する場合は、ビルドファイルの変更が必要です。
次は@<code>{apps.versions.toml}というファイルから@<code>{apps}という名前のカタログを使用する場合の例です。

//list[VersionCatalog7][apps.versions.tomlというファイルから apps というカタログを宣言する（settings.gradle）][groovy]{
dependencyResolutionManagement {
    versionCatalogs {
        apps {
            from(files('gradle/apps.versions.toml'))
        }
    }
}
//}

//list[VersionCatalog8][使用する側は apps という名前で参照することが可能][groovy]{
dependencies {
    implementation apps.androidx.appcompat
}
//}

=== 基本的な宣言と使い方

具体例があるとわかりやすいので、AppCompatライブラリ@<fn>{AppCompat}を依存関係に追加する場合を考えます。
まず、Version Catalogを使わずにbuild.gradleにそのまま依存関係を追加する方法を見ていきましょう。

//footnote[AppCompat][https://developer.android.com/jetpack/androidx/releases/appcompat?hl=ja]

//list[VersionCatalog9][build.gradle（Version Catalogを使わない従来の依存関係の追加）][groovy]{
dependencies {
    implementation "androidx.appcompat:appcompat:1.6.1"
}
//}

次に、Version Catalogを使用する場合を見ていきます。
TOMLファイル内でエイリアスとライブラリのGAV座標（Group, Artifact, Version）を関連付けて宣言します。

//list[VersionCatalog10][libs.versions.toml][toml]{
[libraries]
androidx-appcompat = "androidx.appcompat:appcompat:1.6.1"
//}

使うときには、TOMLファイル内で宣言したエイリアスを参照すればOKです。
@<code>{<カタログ名>.<エイリアス名>}のフォーマットになります。

//list[VersionCatalog11][build.gradle（Version Catalogを使用する依存関係の追加）][groovy]{
dependencies {
    implementation libs.androidx.appcompat
}
//}

==== 有効なエイリアス

エイリアスはアスキー文字で構成され、数字はその後に続く必要があります。
区切り文字には、ダッシュ（@<code>{-}）、アンダースコア（@<code>{_}）、ドット（@<code>{.}）を使用でき、Gradle公式ではダッシュを推奨しています@<fn>{alias-dash}。
宣言したエイリアスから生成されるカタログ（型安全なアクセサ）は、ダッシュ、アンダースコア、ドットのいずれの区切り文字を使用していたとしても自動的にドットに変換されます。
次は、有効なエイリアスと生成されるタイプセーフなアクセサの例です。

//footnote[alias-dash][https://docs.gradle.org/8.1.1/userguide/platforms.html#sub:central-declaration-of-dependencies:~:text=Aliases must consist of a series of identifiers separated by a dash (-%2C recommended)%2C an underscore (_) or a dot (.).]

//table[table1][有効なエイリアスと生成されるアクセサ例]{
有効なエイリアス  生成されるアクセサ
--------------------------------------------
hoge             hoge
hoge-fuga        hoge.fuga 
hoge-fuga1       hoge.fuga1 
hoge.fuga.piyo   hoge.fuga.piyo 
//}

大文字小文字の区別についてですが、Gradle公式ではできれば小文字@<fn>{alias-lowercase}としています。
また、Android公式では依存関係やプラグインの命名規則にケバブケース（例: hoge-fuga）を推奨しています@<fn>{alias-kebabcase1}@<fn>{alias-kebabcase2}。
ただ、Gradle公式ではドットで区切られたサブグループのアクセサを生成したくない場合には大文字と小文字を区別することもオススメしています。
たとえば、カタログファイルに宣言されたアプリのバージョン情報（versionCode, versionName）やAPIレベル（compileSdk, targetSdk, minSdk）を参照する場合などは、サブグループに分かれていないアクセサの方が意味が通りやすいかもしれません。

//footnote[alias-lowercase][https://docs.gradle.org/8.1.1/userguide/platforms.html#sub:central-declaration-of-dependencies:~:text=Identifiers themselves must consist of ascii characters%2C preferably lowercase%2C eventually followed by numbers.]
//footnote[alias-kebabcase1][https://developer.android.com/studio/build/migrate-to-catalogs?hl=ja#:~:text=カタログの依存関係ブロックで推奨される命名規則は、ビルドファイル内でより適切にコード補完を支援するためのケバブケース（androidx-ktx など）です。]
//footnote[alias-kebabcase2][https://developer.android.com/studio/build/migrate-to-catalogs?hl=ja#:~:text=依存関係と同様に、ビルドファイル内でより適切にコード補完を支援するための plugins ブロック カタログ エントリの推奨形式は、ケバブケース（android-application など）です。]

//table[table2][有効なエイリアスと生成されるアクセサ例]{
有効なエイリアス  生成されるタイプセーフなアクセサ
--------------------------------------------
version-code     version.code
versionCode      versionCode 
//}

//image[image1][Android Studioでの補完]{
//}

Android Studioでの補完

また、予約語があるので一部のキーワード（@<code>{extensions}、@<code>{class}など）はエイリアスとして使うことはできません@<fn>{alias-reserved}。

//footnote[alias-reserved][詳細な予約語は公式を参照してください。https://docs.gradle.org/8.1.1/userguide/platforms.html#sub:central-declaration-of-dependencies:~:text=Some keywords are reserved%2C so they cannot be used as an alias. Next words cannot be used as an alias%3A]

=== カタログファイルのセクション

カタログファイルで定義する依存関係やプラグインですが、決まったセクションで宣言する必要があります。
次の4つのセクションがあるので、それぞれ見ていきましょう。

 1. versions: 依存関係やプラグインで参照されるバージョンを定義するセクション
 2. libraries: 依存関係を定義するセクション
 3. bundles: 複数の依存関係を2つにして定義できるセクション
 4. plugins: プラグインを定義するセクション

==== versionsセクション

versionsセクションは、依存関係やプラグインで参照されるバージョンを定義するセクションです。
同じバージョンを参照する複数のエイリアスがある場合にとくに有用で、versionsセクションに共有しているバージョンを定義すれば、1箇所で管理するだけでOKになります。
次は、Lifecycleライブラリ@<code>{lifecycle}での例です。

//footnote[lifecycle][https://developer.android.com/jetpack/androidx/releases/lifecycle?hl=ja]

//list[VersionCatalog12][同じバージョンを繰り返し使用する場合][toml]{
[libraries]
lifecycle-viewmodel-ktx = { module = "androidx.lifecycle:lifecycle-viewmodel-ktx", version = "2.5.1" }
lifecycle-viewmodel-compose = { module = "androidx.lifecycle:lifecycle-viewmodel-compose", version = "2.5.1" }
//}

//list[VersionCatalog13][versionsセクションに繰り返し使用するバージョンを定義した場合][toml]{
[versions]
androidx-lifecycle = "2.5.1"

[libraries]
lifecycle-viewmodel-ktx = { module = "androidx.lifecycle:lifecycle-viewmodel-ktx", version.ref = "androidx-lifecycle" }
lifecycle-viewmodel-compose = { module = "androidx.lifecycle:lifecycle-viewmodel-compose", version.ref = "androidx-lifecycle" }
//}

また、versionsセクションに宣言されたバージョンは依存関係やプラグインで参照できるだけでなく、値そのものをタイプセーフなアクセサ@<code>{<カタログ名>.<セクション名>.<バージョン名>.get()}で利用できます。
そのため、アプリのバージョン情報やAPIレベル要件、Javaのバージョンを定義しておくのも良いでしょう。
ただし、文字列で取得されるので@<code>{targetSDK}など数値で利用する必要がある場合は@<code>{toInteger()}で変換する必要があります。

//list[VersionCatalog14][libs.versions.toml][toml]{
[versions]
app-versionCode = "1"
app-versionName = "1.0.0"

app-compileSdk = "33"
app-minSdk = "24"
app-targetSdk = "33"

java-version = "1.8"
//}

//list[VersionCatalog15][build.gradle][groovy]{
android {
    compileSdk libs.versions.app.compileSdk.get().toInteger()
    compileSdk libs.versions.app.compile.sdk.get().toInteger()

    defaultConfig {
        minSdk libs.versions.app.minSdk.get().toInteger()
        targetSdk libs.versions.app.targetSdk.get().toInteger()

        versionCode libs.versions.app.versionCode.get().toInteger()
        versionName libs.versions.app.versionName.get()
    }

    compileOptions {
        sourceCompatibility libs.versions.java.version.get()
        targetCompatibility libs.versions.java.version.get()
    }

    kotlinOptions {
        jvmTarget = libs.versions.java.version.get()
    }
}
//}

==== librariesセクション

librariesセクションは、依存関係を定義するセクションです。
いろいろな宣言の仕方がありますが、どれも同じ内容です。

//list[VersionCatalog16][いろいろな宣言の仕方][toml]{
[versions]
androidx-appcompat = "1.6.1"

[libraries]
# 書き方 1
androidx-appcompat = "androidx.appcompat:appcompat:1.6.1"
# 書き方 2
androidx-appcompat = { module = "androidx.appcompat:appcompat", version = "1.6.1" }
# 書き方 3
androidx-appcompat = { module = "androidx.appcompat:appcompat", version.ref = "androidx-appcompat" }
# 書き方 4
androidx-appcompat = { group = "androidx.appcompat", name = "appcompat", version = "1.6.1" }
# 書き方 5
androidx-appcompat = { group = "androidx.appcompat", name = "appcompat", version.ref = "androidx-appcompat" }
//}

また、BOMが提供されているライブラリの場合も宣言可能です。
次は、Compose@<fn>{compose}のBOMを使用した例です。

//footnote[compose][https://developer.android.com/jetpack/androidx/releases/compose?hl=ja]

//list[VersionCatalog17][BOMが提供されている依存関係の宣言][toml]{
[libraries]
androidx-compose-bom = { module = "androidx.compose:compose-bom", version = "2023.01.00" }
androidx-compose-material3 = { module = "androidx.compose.material3:material3" }
androidx-compose-ui-tooling-preview = { module = "androidx.compose.ui:ui-tooling-preview" }
androidx-compose-ui-tooling = { module = "androidx.compose.ui:ui-tooling" }
//}

==== bundlesセクション

bundlesセクションは、複数の依存関係を1つにして定義できるセクションです。
よく一緒に使用するライブラリをまとめておけば、個別に複数の依存関係を追加する必要がなくなります。
たとえば、@<code>{lifecycle-viewmodel-ktx}と@<code>{lifecycle-viewmodel-compose}を一緒に使用する場合には次のように定義しておくと便利です。

//list[VersionCatalog18][libs.versions.toml][toml]{
[versions]
androidx-lifecycle = "2.5.1"

[libraries]
lifecycle-viewmodel-ktx = { module = "androidx.lifecycle:lifecycle-viewmodel-ktx", version.ref = "androidx-lifecycle" }
lifecycle-viewmodel-compose = { module = "androidx.lifecycle:lifecycle-viewmodel-compose", version.ref = "androidx-lifecycle" }

[bundles]
lifecycle = ["lifecycle-viewmodel-ktx", "lifecycle-viewmodel-compose"]
//}

//list[VersionCatalog19][bundlesを使用しない場合（2つの依存関係を追加するには2行記載する必要がある）][groovy]{
dependencies {
    implementation libs.lifecycle.viewmodel.ktx
    implementation libs.lifecycle.viewmodel.compose
}
//}

//list[VersionCatalog20][bundlesを使用する場合（2つの依存関係を追加するのに1行の記載でOK）][groovy]{
dependencies {
    implementation libs.bundles.lifecycle
}
//}

==== pluginsセクション

pluginsセクションは、プラグインを定義するセクションです。
ライブラリがGAV座標で表されるのに対し、Gradleプラグインはidとバージョンで識別されます。

//list[VersionCatalog21][libs.versions.toml][toml]{
[plugins]
android-application = { id = "com.android.application", version = "8.0.1" }
//}

//list[VersionCatalog22][libs.versions.toml][groovy]{
plugins {
    alias(libs.plugins.android.application)
}
//}

== 依存関係自動更新ツールRenovateの導入

Android Studio GiraffeからTOMLファイルで定義した依存関係が古い場合には警告表示がされるようになり、以前より依存関係の更新に気づきやすくなりました。
しかし、プロジェクトの依存関係更新を自動化するツールを導入するとより効率的に対応できます。
今回は、Version Catalogに対応しているRenovateの導入について紹介します。

=== Renovateの導入

GitHubを利用している場合は、用意されているRenovateのGitHub Appをインストール@<fn>{renovate-install}します。
@<code>{renovate.json}ファイルを追加するプルリクエストが自動で作成されるので、そのプルリクエストをマージすればRenovateが有効になります。

//footnote[renovate-install][そのほかの導入方法は公式を参照してください。https://www.mend.io/renovate/]

//image[image2][RenovateのGitHub App]{
//}

//image[image3][Renovate導入時の自動で作成されるプルリクエスト]{
//}

その後は、更新できるバージョンがある場合にRenovateが自動でプルリクエストを作成してくれます。

//image[image4][更新できるバージョンがある場合に自動で作成されるプルリクエスト]{
//}

=== Renovateの設定

Renovateの設定項目は数多くある@<fn>{renovate-setup-options}ため、今回は@<code>{schedule}のみ説明いたします。
Renovateは更新できるバージョンがある場合にプルリクエストを自動で作成してくれますが、都度作成されると煩わしく感じたり、休日に作成されると不都合になったりする場合もあります。
その際は@<code>{schedule}項目でプルリクエストの作成タイミングを設定してあげるとよいでしょう。
たとえば、月曜日の10:00~17:00の間のみに限定できます。

//footnote[renovate-setup-options][Renovateの詳細な設定項目は公式を参照してください。https://docs.renovatebot.com/configuration-options/]
//footnote[renovate-setup-schedule][https://docs.renovatebot.com/configuration-options/#schedule]

//list[VersionCatalog23][renovate.json][json]{
{
  "$schema": "https://docs.renovatebot.com/renovate-schema.json",
  "extends": [
    "config:base"
  ],
  "timezone": "Asia/Tokyo",
  "schedule": ["after 10am and before 5pm on monday"]
}
//}

設定内容はプルリクエスト上でも確認できます。

//image[image5][プルリクエストにも設定内容が反映される]{
//}

== おわりに

Version Catalogを利用することで依存関係やプラグインのバージョンの一元管理が容易になるだけでなく、Android Studioでコード補完や定義ジャンプが効くのは大きなメリットだと感じています。
さらに、Renovateなどの依存関係自動更新ツールを導入すれば、定期的なバージョン更新も叶うので煩雑さから解放されるのが嬉しいですね。
