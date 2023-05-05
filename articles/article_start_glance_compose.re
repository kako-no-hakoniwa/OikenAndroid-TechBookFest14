= Glanceで遊ぼう

== Glanceって何？

Glanceとは、AndroidのウィジェットをJetpackComposeの記法で書くことができるライブラリです。
2023年5月現在はまだアルファ版ですが、従来の作り方よりも簡単にウィジェットを作ることが可能です。

=== そもそもウィジェット開発って
ウィジェットはAndroidデバイスのホーム画面上に配置できる小さなアプリケーションの一部で、ユーザーにリアルタイムな情報を提供したり、アプリの一部機能を切り出して利用できるようにしたりします。
従来のウィジェット開発では、RemoteViewsという仕組みを用いて実装する必要がありましたが、これには制約が多く、開発が煩雑になることがありました。

=== JetpackComposeで書けるとはいえ、制限もある
Glanceを使うと、ウィジェット開発においてJetpack Composeの記法を利用できます。「記法を利用できる」というのは、Composable関数として記述することが可能なことを指していますが、普段のJetpack Composeそのままの考え方で書けるというわけではありません。
例えば普段馴染みのあるModifierはGlanceでは使えず、代わりにGlanceModifierというものを使います。
また、状態の変更に応じて表示内容を差分更新してくれる仕組み（Recompose）はなく、画面全体を更新するイメージです。

これらの前提はありつつも、Glanceを使ったウィジェット開発は、従来のRemoteViewsを直接使った方法と比べてシンプルにウィジェットを作成できます。

== 環境構築
プロジェクトのbuild.gradleファイルにGlanceの依存関係を追加します。（@<list>{build.gradle})
//list[build.gradle][build.gradle]{
dependencies {
    implementation "androidx.glance:glance-appwidget:1.0.0-alpha05"
}

android {
    buildFeatures {
        compose true
    }

    composeOptions {
        // 使用しているKotlinバージョンと互換性を持ったバージョンを指定。（筆者環境はKotlinバージョン1.8.20を使用。）
        kotlinCompilerExtensionVersion = "1.4.5" 
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
}
//}
※参照 https://developer.android.com/jetpack/androidx/releases/compose-kotlin

Glanceは専用のComposeを使うため、通常のJetpack Composeの依存関係の追加は不要です。

== Glanceを利用したシンプルなウィジェットの構築 Hello World
まずは画面更新などを考えない、シンプルなウィジェットでHello Worldしてみましょう。

GlanceではGlanceAppWidgetとGlanceAppWidgetReceiverというクラスが主要な登場人物です
これらを継承したクラスを実装していきます。

=== GlanceAppWidgetの作成
まずは、GlanceAppWidgetクラスを継承したカスタムウィジェットクラスを作成します。
content()メソッドにおなじみの@Composableアノテーションがついており、ここにウィジェットのUIを記述します。

//list[GlanceAppWidget][GlanceAppWidgetを継承してComposableを記述]{
import androidx.compose.runtime.Composable
import androidx.glance.appwidget.GlanceAppWidget
import androidx.glance.text.Text

class GlanceAppWidgetSample : GlanceAppWidget() {

    @Composable
    override fun Content() {
        Text(text = "Hello, Glance!")
    }
}
//}
通常のJetpack Composeも導入しているプロジェクトの場合、使用しているコンポーネントがGlance用のものになっているかどうかに注意が必要です。
今回の場合、Textコンポーネントがandroidx.glance.text.Textであることを確認してください。


=== GlanceAppWidgetReceiverの作成
GlanceAppWidgetReceiverはウィジェットのアップデートイベントを始めとした各種イベントを受け取るためのクラスです。
BroadcastReceiverを継承しています。
ウィジェットを表示させるためには最低限以下の記述が必要です。先程作成したGlanceAppWidgetを継承したクラスを指定します。
//list[GlanceAppWidgetReceiver][GlanceAppWidgetReceiverを継承しglanceAppWidgetをoverride]{
import androidx.glance.appwidget.GlanceAppWidget
import androidx.glance.appwidget.GlanceAppWidgetReceiver

class GlanceAppWidgetReceiverSample : GlanceAppWidgetReceiver() {
    
    override val glanceAppWidget: GlanceAppWidget
        get() = GlanceAppWidgetSample()
}
//}

=== メタデータファイルの作成
※属性は公式ドキュメントを参照 https://developer.android.com/develop/ui/views/appwidgets#AppWidgetProviderInfo

ウィジェットの設定や動作に関する情報を定義するためにXMLでメタデータファイルを作成する必要があります。
本当にHello Worldとしてただウィジェットを表示するだけであれば属性は一つも指定しなくても問題ありませんが、ここではある程度最低限の指定をしていきましょう。
//list[metadata][widget_meta_data]{
<?xml version="1.0" encoding="utf-8"?>
<appwidget-provider xmlns:android="http://schemas.android.com/apk/res/android"
    android:previewImage="@mipmap/ic_launcher" ← ウィジェット追加時のプレビュー画像
    android:minWidth="80dp"  ← ウィジェットデフォルト幅
    android:minHeight="80dp" ← ウィジェットデフォルト高さ
    android:targetCellWidth="2" ← Android12以降でのデフォルトのグリッド幅
    android:targetCellHeight="2" ← Android12以降でのデフォルトのグリッド高さ
    android:resizeMode="horizontal|vertical" ←ウィジェット長押しでリサイズ可能な方向
     />
//}


=== AndroidManifestに追記

前述の通り、GlanceAppWidgetReceiverはBroadcastReceiverを継承しているため、忘れずにAndoridManifestにReceiver登録します。
また先ほど作成したメタデータファイルもあわせて記述します。

//list[AndroidManifest][AndroidManifest.xml]{
<receiver
    android:name=".GlanceAppWidgetReceiverSample"
    android:exported="false">
    <intent-filter>
        <action android:name="android.appwidget.action.APPWIDGET_UPDATE" />
    </intent-filter>
    <meta-data
        android:name="android.appwidget.provider"
        android:resource="@xml/widget_meta_data" />
</receiver>
//}

=== ビルドしてみよう
ビルドが完了したら、実機またはエミュレーターでアプリを起動し、ホーム画面に作成したウィジェットを追加してみましょう。正しく設定されていれば、ウィジェットに"Hello, Glance!"というテキストが表示されるはずです。

ここに画像
Hello Glance!!

== ウィジェットの表示をカスタムする方法
ここからはウィジェットの表示をカスタムしていきましょう。

=== GlanceModifier
通常のJetpack ComposeでUIを作成する際、Modifierを使用して要素のレイアウトやスタイルを変更することが一般的です。
ただし、GlanceではModifierは直接利用できず、代わりにGlanceModifierというものを使います。基本的な使い方はModifierと同じなのですが、用意されている属性に限りがあるため、通常のModifierと同じようにレイアウトを組めない場合があります。

☆ 参照 https://developer.android.com/reference/kotlin/androidx/glance/GlanceModifier#extension-functions_1

また、通常のComposeではRowであればhorizontalArrangement、ColumnであればverticalArrangementと、引数にArrangementを取りますがGlanceのRowとColumnはhorizontalもverticalもAlignmentしか引数に取りません。
そのためArrangement.SpaceArroundやArrangement.SpaceBetweenなどを指定した要素の配置は現状できません。
RowScopeで用意されているGlanceModifierにweight()はなくdefaultWeight()しかいないため、自由にweightを指定することができません。

などといった制約はありますが、それでも

ここでコラム的な枠
☆間違ってModifierを使ったりするとこうなる
エラーの画像
Modifierを始めとした通常のComposeのコンポーネントを誤って使ってしまった場合、エラーメッセージが表示されることがあります。このような場合は、Glanceで利用できるコンポーズ要素や属性について調べ、適切な方法で表示をカスタマイズするようにしましょう。

=== リスト表示
LazyColumnが使えます。（ただしパッケージはandroidx.glance）
LazyRowはまだありません。

=== 画像の表示
普通のComposeと同じように書けます（ただし使うものはandroidx.glance.ImageKtです）
//list[ImageKt][画像の表示]{
Image(
    provider = ImageProvider(
        R.drawable.ic_launcher_foreground
    ),
    contentDescription = null,
    modifier = GlanceModifier.size(90.dp)
)
//}


=== クリックアクションの追加
GlanceModifier.clickableを使うことでクリック時の処理を指定することができます。
clickableはandroidx.glance.action.ActionというInterfaceを引数に取るため、Actionを実装することで処理を記述できます。

==== Activityの起動
いくつかのActionは予め用意されています。
Activityの起動にはactionStartActivity()を使います。
//list[Activityの起動][Activityの起動]{
Box(
    modifier = GlanceModifier.clickable(actionStartActivity<MainActivity>())
    ) {
    }
//}

またBroadcast送信にはactionSendBroadcast()、Serviceの起動にはactionStartServece()が用意されています。


==== 独自Actionの起動
独自のクリックアクションを定義するには、行いたい処理をActionCallbackを継承したクラスに記述し、そのクラスを actionRunCallback に型パラメータとして渡すことで実現できます。
//list[独自のクリックアクション][独自のクリックアクションの実装]{
Box(
    modifier = GlanceModifier.clickable(actionRunCallback<SampleAction>())
    ) {
    }

class SampleAction : ActionCallback {
    override suspend fun onAction(
        context: Context,
        glanceId: GlanceId,
        parameters: ActionParameters
    ) {
        // ここに行いたい処理
    }
}
//}

== ウィジェットの表示を更新する
ウィジェットの表示を更新するためには、まず表示要素となるデータの更新をした上で画面の更新処理を呼び出す必要があります。

=== ウィジェットの更新
通常のJetpack Composeでは、引数にとったデータの変更を自動で検知しRecomposeされますが、Glanceでは手動で画面の更新を呼び出す必要があります。
GlanceAppWidgetに用意されているupdate関数を使用します。（GlanceAppWidgetを継承して自分で作成したクラスのupdate()を呼び出します。）
//list[手動更新update(context, id)][update(context, id)]{
GlanceAppWidgetSample().update(context, id)
//}

引数のidはウィジェットの識別IDです。ウィジェットは複数作成することができ、それぞれにidが存在します。
例えば前述のActionCallBackのonAction()には引数としてidが渡ってくるため、そのidを使ってタップされたウィジェットのみ更新が可能です。
idを意識しない場合はupdateAll(context)も使えます。すべてのウィジェットに反映すべきデータの更新後はupdateAll()を使う、など使い分けることができます

//list[手動更新updateAll(context)][updateAll(context)]{
GlanceAppWidgetSample().updateAll(context)
//}

=== データの更新
表示するためのデータの更新には、DataStore*の仕組みが使われます。
DataStore自体にはPreferences DataStoreとProto　DataStoreの2種類がありますが、そのどちらもGlanceで利用できます。
具体的には、androidx.glance.appwidget.state.GlanceAppWidgetStateKt#updateAppWidgetState()というメソッドが引数違いでPreferences DataStoreとProto　DataStore版のそれぞれが用意されています。

==== Preferences DataStoreを使ってウィジェットの状態を更新する
updateAppWidgetState(Preferences DataStore版)は以下の引数を取ります。
//list[updateAppWidgetState(Preferences DataStore版)][updateAppWidgetState(Preferences DataStore版)]{
suspend fun updateAppWidgetState(
    context: Context,
    glanceId: GlanceId,
    updateState: suspend (MutablePreferences) -> Unit,
) {
    ・・・
}
//}

Preferences DataStoreを使ってデータ更新を行うシンプルなサンプルコードは以下のようになります。
//list[Preferences DataStoreを使ったデータの更新][Preferences DataStoreを使ったデータの更新]{
private suspend fun updateWidget(context: Context){
    val manager = GlanceAppWidgetManager(context)
    val glanceIds = manager.getGlanceIds(GlanceAppWidgetSample::class.java)
    glanceIds.forEach { glanceId ->
        updateAppWidgetState(context, glanceId) { prefs ->
            prefs[stringPreferencesKey("key_name")] = "セットしたい文字列"
        }
    }
    GlanceAppWidgetSample().updateAll(context)
}
//}

セットしたデータはCompositionLocalから取得できます。
//list[Preferences DataStoreを使ったデータの取得][Preferences DataStoreを使ったデータの取得]{
val prefs = currentState<Preferences>()
val string: String = prefs[stringPreferencesKey("key_name")]
//}

==== Proto DataStoreを使ってウィジェットの状態を更新する
updateAppWidgetState(Proto DataStore版)は以下の引数を取ります。

//list[updateAppWidgetState(Proto DataStore版)][updateAppWidgetState(Proto DataStore版)]{
suspend fun <T> updateAppWidgetState(
    context: Context,
    definition: GlanceStateDefinition<T>,
    glanceId: GlanceId,
    updateState: suspend (T) -> T,
): T {
    ・・・
}
//}
GlanceStateDefinition<T>は、型パラメータに保存するデータ型を取るinterfaceです。
そのため、GlanceStateDefitinitionを実装したクラスを作成する必要があります。

//list[GlanceStateDefitinitionの実装][GlanceStateDefitinitionの実装]{
object GlanceAppSampleStateDefinition : GlanceStateDefinition<SampleState> {

    private const val DATA_STORE_FILENAME = "sampleState"
    private val Context.datastore by dataStore(DATA_STORE_FILENAME, SampleStateSerializer)

    override suspend fun getDataStore(context: Context, fileKey: String): DataStore<SampleState> {
        return context.datastore
    }

    override fun getLocation(context: Context, fileKey: String): File {
        return context.dataStoreFile(DATA_STORE_FILENAME)
    }
}

@Serializable
sealed interface SampleState {
    @Serializable
    object Loading : SampleState

    @Serializable
    data class Success(
        val displayString: String,
    ) : SampleState

    @Serializable
    data class Error(val message: String) : SampleState
}
//}

Proto DataStoreを使って、先程定義したSampleState.Success状態にデータ更新を行うシンプルなサンプルコードは以下のようになります。
通常のJetpackComposeにおけるUI状態更新でよく見るような書き方ができることがわかるかと思います。
//list[Proto DataStoreを使ったデータの更新][Proto DataStoreを使ったデータの更新]{
private suspend fun setWidgetStateSuccess(context: Context) {
    val manager = GlanceAppWidgetManager(context)
    val glanceIds = manager.getGlanceIds(GlanceAppWidgetSample::class.java)
    glanceIds.forEach { glanceId ->
        updateAppWidgetState(
            context = context,
            definition = GlanceAppSampleStateDefinition,
            glanceId = glanceId,
            updateState = { SampleState.Success(displayString = "セットしたい文字列") }
        )
    }
    GlanceAppWidgetSample().updateAll(context)
}
//}

セットしたデータを取り出すにはstateDefinitionに先ほどのGlanceStateDefitinitionを実装したクラスを指定し、CompositonLocalから取得できます。
//list[Proto DataStoreを使ったデータの取得][Proto DataStoreを使ったデータの取得]{
class GlanceAppWidgetSample : GlanceAppWidget() {

    override val stateDefinition = GlanceAppSampleStateDefinition
    @Composable
    override fun Content() {
        val sampleState = currentState<SampleState>()
        ・・・
    }
}
//}

=== 更新トリガー
データ更新の仕方を見てきました。続いて、更新処理のトリガーを引く方法を見ていきます。

○章でメタデータファイルとしてwidget_meta_data.xmlを作成しましたが、属性の一つにupdatePeriodMillisがあり、更新の時間を指定することができます。指定した時間の周期でGlanceAppWidgetReceiverのonUpdate()がコールされるため、更新処理をonUpdate()内に記述することで定期的な更新が可能です。
しかし、updatePeriodMillisの最小値は1800000（=30分）に制限されているため、それより高頻度のデータ更新が必要な場合は、WorkManagerやAlarmManagerを使用して周期的処理を実装する必要があります。

データの更新タイミングはアプリの要件や仕様によるところが大きく、またGlanceに限った話ではありませんが主に以下の使い分けが考えられます。
 * 手動更新のみでOK
 ** → クリックイベントで更新
 * 一日1回〜30分に1回の頻度で更新
 ** → updatePeriodMillisで指定
 * 30分〜15分に1回の頻度で更新
 ** → WorkManagerを使用（WorkManagerで指定できる繰り返し処理の最短周期は15分のため）
 * 15分〜5分に1回の頻度で更新
 ** → AlarmManager（AlarmManagerで指定できる繰り返し処理の最短周期は5分のため）
 * 5分に1回より高頻度
 ** → CoroutinesのDelayなどでWhile的に更新

そもそも高頻度で更新が必要なものは、バッテリー消費などの観点からウィジェットとして表示するのに相応しいのかどうか？というのはおいておいて、頻度に応じてトリガーを使い分けましょう。
ただ、いずれの場合にもユーザーが任意のタイミングで情報を更新できるように手動更新ボタンを設置しておくのが望ましいでしょう。
updatePeriodMillisによる更新も、正確にこの周期で実行されることが保証されているわけではないため注意が必要です。

==== onUpdate(), onEnabled(), onDisabled()
updatePeriodMillisで指定した周期でGlanceAppWidgetReceiverのonUpdate()が呼ばれると説明しましたが、他にもウィジェットの状態に応じて呼ばれるメソッドがあるため説明します。

===== onUpdate()
ウィジェットがホーム画面に追加されたとき、updatePeriodMillisで指定した周期

===== onEnabled()
ウィジェットがホーム画面に追加されたとき

===== onDisabled()
ウィジェットがホーム画面から削除されたとき

これらはGlance特有ではなく従来のウィジェット実装と同様のため、既存のドキュメントを参照するのが正確です。
https://developer.android.com/guide/topics/appwidgets?hl=ja#AppWidgetProvider


== ウィジェットのサイズに応じたレイアウト変更
レスポンシブなレイアウトであればウィジェットのサイズが変わっても動的に対応できますが、
ウィジェットのサイズによって、表示するレイアウトごと（Composable関数ごと）変更したいケースの対応方法です。

以下のように、GlanceAppWidgetを継承したクラスのsizeModeをoverrideしDpSizeのセットを指定することで現在のサイズを取得することができます。
//list[ウィジェットのサイズに応じたレイアウト変更][ウィジェットのサイズに応じたレイアウト変更]{
class GlanceAppWidgetSample : GlanceAppWidget() {

    companion object {
        private val smallMode = DpSize(160.dp, 160.dp)
        private val mediumMode = DpSize(200.dp, 200.dp)
        private val largeMode = DpSize(240.dp, 240.dp)
    }

    override val sizeMode: SizeMode = SizeMode.Responsive(
        setOf(smallMode, mediumMode, largeMode)
    )

    @Composable
    override fun Content() {
        val size = LocalSize.current
        when (size) {
            smallMode -> {
                Text(text = "smallMode")
            }
            mediumMode -> {
                Text(text = "mediumMode")
            }
            largeMode -> {
                Text(text = "largeMode")
            }
        }
    }
}
//}

== リファレンス

GlanceはRemoteViewsをラップしているものなので、内部の仕組みとしては既存のウィジェットの仕組みが生きています。
そのため従来のウィジェットのドキュメントも有用です。
https://developer.android.com/develop/ui/views/appwidgets

公式のウィジェットサンプルアプリです。
特にProto DataStoreを利用したデータ更新やウィジェットのサイズに応じた画面表示の切り替えなどはブログ記事などでもあまり言及されていないため、実装時にはこれを見るべきです。
https://github.com/android/user-interface-samples/tree/main/AppWidget



== コードを見てみる

どうやってupdateを呼ぶだけで画面を更新している？？
従来のWidgetでは

appWidgetManager.updateAppWidget() でremoteViewをセットする

同じようにGlanceAppWidgetのupdate()メソッドの中のcompose()がRemoteViewを返す実装になっている

なんかこの中でサイズとかみてやっているな、これって従来のWidgetよりも手厚いのかどうなのか

== おまけ Glanceでミニゲーム

電池消費や画面描画の遅さ、またホーム画面でそれを行う意義などを考えるとまったく実用的ではないけどロマンです


レベルは自分でセットしたウィジェットのサイズで調整？？

===　ゲーム開発の概要
まずは、Glanceを使って簡単なミニゲームを開発する方法を考えていきましょう。ゲームのシンプルな要素を実装し、遊べる程度のものを目指します。

===　ゲームの構造
ルール 敵に触れるとダメージを受ける 別に敵はビームとかを持たない
敵はドロイド君でいいか。。。？
シュートもできるようにする？？

ウィジェット上で動作する簡単なミニゲームを作成するため、以下のような構造を考えます。

自機の移動
敵の生成と移動
衝突判定
スコアの表示
===　自機の移動
コントロールボタンを設置し、ボタンをクリックしたときに自機の位置を更新することで、自機の移動を実現します。

===　敵の生成と移動
一定時間ごとに、画面上部からランダムな位置で敵を生成し、敵を下方向に移動させます。


update5回ごとに1回 敵の生成

毎回のupdateで位置を一つずらす

毎回のupdateで、すでに一番下に到達している場合はアイテム破棄＆特典加算

シューティング要素ありなら、毎回のupdateで位置を一つ上にずらす そこに敵がいれば破棄。


=== 時間の経過にともない難しくなる仕組み
敵を増やす


=== 回復アイテム



===　衝突判定
自機と敵が衝突した場合、ゲームオーバーとなります。衝突判定は矩形同士の判定を行うことで実現できます。

===　スコアの表示
ウィジェット上にスコアを表示し、敵を避けた回数に応じてスコアが増加する仕組みを実装します。

=== 開始画面とゲームオーバー画面

===　ゲームの最適化
ウィジェット上で動作するゲームのため、電池消費や画面描画の遅さを考慮し、最適化を行います。

今回の要素数ではあまり問題になりませんが、SharedではなくProtoDatastoreを使う
例えば100×100とか。


===　ゲームの完成
ここまでで、Glanceを使ったミニゲームが完成しました。ウィジェット上で遊べるシンプルなゲームが実現できました。

===　まとめ
この記事では、Jetpack ComposeのGlanceを使ったウィジェット開発について紹介しました。Glanceを使えば、Jetpack Composeの記法でウィジェットを作成できるため、開発者にとってより効率的な開発が可能です。




=== 要件
これまでの総集編 メタ的な要素

これから開発




=== トラブルシュート

Error in Glance App Widget
 java.lang.IllegalArgumentException: Cannot find container Row with 12 children

 ウィジェットの幅をはみ出るようなコンテンツをセットすると発生。?Rで11個以上で発生した とおもったけどサイズとかなのかなやっぱり
 とりあえず10子でdefaultWeightを使うと良さそうな感じ











 === ぼやき
 うーんdataStoreでたくさんの値をまとめて効率的に取れるのかな`

 Widget自身でupdateは呼べる？？
 呼べるとして、今回はスタートボタンで初期化などを走らせたいからそれをBroadcastで受け取って処理するのは間違っていない
とは思うけど

右左ボタンの中でupdateを呼ぶと、自位置だけ固まるような挙動になった。これはdataStoreの更新の前に自身をcomposeし直すから？？？

宇宙背景 https://sozaino.site/archives/7558
https://sozaino.site/archives/8580

https://sozaino.site/archives/4807




手動更新ボタンをおいておくのが良さそう。これは画面の表示を更新する必要があるのであればどんなウィジェットでも。