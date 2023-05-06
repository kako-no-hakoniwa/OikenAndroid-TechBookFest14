= Glanceで遊ぼう

この章ではAndroidのウィジェット開発を簡単にしてくれるJetpack Glanceについて説明します。

== Glanceって何？
Glanceは、Androidデバイスのホーム画面上に配置できるウィジェットをJetpack Composeの記法を使って開発できるライブラリです。

従来のウィジェット開発では、RemoteViewsを直接用いて実装する必要があり、またクリックイベントはすべてIntentを発行して制御する必要があるなど、開発が煩雑になる部分がありました。
Glanceでは、ComposableをRemoteViewsに変換してくれるため、直接RemoteViewsを意識する必要がなく、クリックイベントもコールバックの形で書けるなど、実装が容易になっています。

現在（2023年5月現在）はまだアルファ版ですが、従来の作り方よりも簡単にウィジェットを作成することが可能です。

=== Jetpack Composeの記法を使えるとはどういうことか
Glanceを使うことで、ウィジェット開発においてJetpack Composeの記法を活用することができます。
ただし、「記法を利用できる」とは、Composable関数としての記述が可能であるという意味であり、通常のJetpack Composeと完全に同じ方法で記述できるわけではありません。

例えば、一般的によく使われるModifierはGlanceでは使用できず、その代わりにGlanceModifierが提供されています。
また、表示内容の差分更新を行うRecomposeのような仕組みは存在せず、イベントに基づいて画面全体を更新するという考え方が適用されています。

しかしこれらの前提を理解した上で、既にJetpack Composeを導入しているプロジェクトでは同様の記述方法でコーディングができるという点は大変有益です。
さらに、従来のウィジェット開発経験がなくても簡単に記述することができるという利点があります。

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

== Glanceを利用したシンプルなウィジェットの構築：Hello World
まずは、画面更新などの要素を考慮せず、シンプルなウィジェットを用いてHello Worldを実現してみましょう。

Glanceでは、GlanceAppWidgetとGlanceAppWidgetReceiverというクラスが主要な役割を担っています。これらのクラスを継承したカスタムクラスを実装していくことで、ウィジェットを作成していきます。


=== GlanceAppWidgetの作成
まずはじめに、GlanceAppWidgetクラスを継承したカスタムウィジェットクラスを作成しましょう。content()メソッドには、おなじみの@Composableアノテーションが付与されており、ここにウィジェットのUIを記述します。

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
通常のJetpack Composeも導入しているプロジェクトでは、使用しているコンポーネントがGlance用のものであるかどうか注意が必要です。
今回の例では、Textコンポーネントがandroidx.glance.text.Textであることを確認してください。

=== GlanceAppWidgetReceiverの作成
GlanceAppWidgetReceiverは、ウィジェットのアップデートイベントをはじめとした各種イベントを受け取るためのクラスであり、これはBroadcastReceiverを継承しています。
ウィジェットを表示させるためには、最低限次の記述が必要です。先ほど作成したGlanceAppWidgetを継承したクラスを指定してください。

//list[GlanceAppWidgetReceiver][GlanceAppWidgetReceiverを継承しglanceAppWidgetをoverride]{
import androidx.glance.appwidget.GlanceAppWidget
import androidx.glance.appwidget.GlanceAppWidgetReceiver

class GlanceAppWidgetReceiverSample : GlanceAppWidgetReceiver() {

    override val glanceAppWidget: GlanceAppWidget
        get() = GlanceAppWidgetSample()
}
//}


=== メタデータファイルの作成
※属性については公式ドキュメントを参照してください：https://developer.android.com/develop/ui/views/appwidgets#AppWidgetProviderInfo

ウィジェットの設定や動作に関する情報を定義するために、XMLでメタデータファイルを作成する必要があります。
単にウィジェットを表示するだけであれば属性を指定せずとも問題ありませんが、ここでは基本的な要素の指定を行いましょう。

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

前述の通り、GlanceAppWidgetReceiverはBroadcastReceiverを継承しているため、忘れずにAndoridManifestにReceiver登録をしましょう。
また、先ほど作成したメタデータファイルもあわせて記述します。

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

Glanceを利用してシンプルなウィジェットを構築する方法を解説しました。これを基本として、さらに複雑なウィジェットや独自の機能を追加していくことができます。

== ウィジェットの表示をカスタムする方法
ここからはウィジェットの表示をカスタムしていきましょう。

=== GlanceModifier
通常のJetpack Composeでは、UIの作成においてModifierを使って要素のレイアウトやスタイルを変更するのが一般的です。しかし、GlanceではModifierを直接使用することができず、代わりにGlanceModifierを使用します。
基本的な使い方はModifierと同じですが、利用できる属性に制限があるため、通常のModifierと同じようなレイアウトは組めないことがあります。

☆ 参照: https://developer.android.com/reference/kotlin/androidx/glance/GlanceModifier#extension-functions_1

さらに、通常のComposeではRowでhorizontalArrangement、ColumnでverticalArrangementという引数を取りますが、GlanceのRowとColumnはhorizontalとverticalの引数としてAlignmentしか取りません。
そのため、Arrangement.SpaceAroundやArrangement.SpaceBetweenなどを指定して要素を配置することは現状できません。
また、RowScopeで提供されているGlanceModifierにはweight()がなく、defaultWeight()しかないため、自由にweightを指定することができません。

といったこれらの制約はありますが、基本的なレイアウトとスタイリングは実現可能です。

ここでコラム的な枠
☆間違ってModifierを使ったりするとこうなる
エラーの画像
Modifierを始めとした通常のComposeのコンポーネントを誤って使ってしまった場合を含め、
エラー時にはウィジェットにエラーメッセージが表示されます。このような場合は、Glanceで利用できるコンポーズ要素や属性について調べ、適切な方法で表示をカスタマイズするようにしましょう。


=== リスト表示
GlanceではLazyColumnが使用できますが、パッケージは androidx.glance になります。
これにより、縦方向のリスト表示を実現することができます。

ただし、現時点ではLazyRowは利用できないため注意が必要です。

=== 画像の表示
Glanceでは通常のComposeと同様に画像を表示することができます。（ただし使うものはandroidx.glance.ImageKtです）
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
いくつかのアクションが予め用意されており、Activityの起動には actionStartActivity() を使います。

//list[Activityの起動][Activityの起動]{
Box(
    modifier = GlanceModifier.clickable(actionStartActivity<MainActivity>())
    ) {
    }
//}

さらに、Broadcastの送信には actionSendBroadcast() を、Serviceの起動には actionStartService() を使用できます。
これらのアクションを使って、ウィジェットをタップした際にアプリケーション内でさまざまな操作を実行することができます。

==== 独自Actionの起動
独自のクリックアクションを定義するには、行いたい処理をActionCallbackを継承したクラスに記述し、そのクラスを actionRunCallback に型パラメータとして渡すことで実現できます。
//list[独自のクリックアクション][独自のクリックアクションの実装]{
Box(
    modifier = GlanceModifier.clickable(actionRunCallback<SampleAction>())
    ) {
        ・・・
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
updateAppWidgetState（Preferences DataStore版）は次の引数を取ります。
//list[updateAppWidgetState(Preferences DataStore版)][updateAppWidgetState（Preferences DataStore版）]{
suspend fun updateAppWidgetState(
    context: Context,
    glanceId: GlanceId,
    updateState: suspend (MutablePreferences) -> Unit,
) {
    ・・・
}
//}

Preferences DataStoreを使ってデータ更新を行うシンプルなサンプルコードは次のとおりです。
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
updateAppWidgetState（Proto DataStore版）は次の引数を取ります。

//list[updateAppWidgetState(Proto DataStore版)][updateAppWidgetState（Proto DataStore版）]{
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

Proto DataStoreを使って、先ほど定義したSampleState.Success状態にデータ更新を行うシンプルなサンプルコードは次のとおりです。
通常のComposeにおけるUI状態更新でよく見るような書き方ができます。
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
これまでデータの更新方法を説明してきました。次に、更新処理をトリガーする方法について見ていきましょう。

○章で、widget_meta_data.xmlというメタデータファイルを作成しました。
その中にある属性の一つであるupdatePeriodMillisを使用して、ウィジェットの更新周期を指定できます。
指定した周期でGlanceAppWidgetReceiverのonUpdate()が呼び出されるため、更新処理をonUpdate()内に記述することで定期的な更新が実現できます。
ただし、updatePeriodMillisの最小値は1800000（=30分）に制限されており、より高頻度のデータ更新が必要な場合は、WorkManagerやAlarmManagerを使用して周期的な処理を実装する必要があります。

データの更新タイミングはアプリの要件や仕様に依存しますが、一般的に次のような使い分けが考えられます。

 * 手動更新のみでOK
 ** クリックイベントで更新
 * 一日1回〜30分に1回の頻度で更新
 ** updatePeriodMillisで指定
 * 30分〜15分に1回の頻度で更新
 ** WorkManagerを使用（WorkManagerで指定できる繰り返し処理の最短周期は15分のため）
 * 15分〜5分に1回の頻度で更新
 ** AlarmManager（AlarmManagerで指定できる繰り返し処理の最短周期は5分のため）
 * 5分に1回より高頻度
 ** CoroutinesのDelayなどを使用してループ処理的に更新

updatePeriodMillisによる更新は、指定された周期で正確に実行されることが保証されていない*ため、注意が必要です。
高頻度の更新が必要なウィジェットでは、バッテリー消費も考慮して適切なトリガーを選択しましょう。

また、どの場合でも、ユーザーがいつでも情報を更新できるように手動更新ボタンを設置することが望ましいです。

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
レスポンシブなレイアウトであればウィジェットのサイズが変わっても動的に対応できますが、それでは適切な表示を維持できない場合や、表示する要素数を変えたい場合があります。
ここではウィジェットのサイズによって表示するレイアウトごと（Composable関数ごと）変更する方法を説明します。

次のように、GlanceAppWidgetを継承したクラスのsizeModeをoverrideしDpSizeのセットを指定することで、CompositionLocalから現在のサイズを取得することができます。
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

== おまけ Glanceでミニゲーム

ここまでの内容を元に、ウィジェットでミニゲームを作成してみました。
このミニゲームは、前方から次々に降ってくるAndroidロボット*を集めるゲームです。
Androidロボットのイメージは、Androidのロゴマークでよく使われているものです。
https://developer.android.com/distribute/marketing-tools/brand-guidelines?hl=ja#android_robot

ただし、ウィジェットでミニゲームを作成すること自体は、バッテリー消費の問題や画面描画の遅さ、またホーム画面でそれを行う意義などを考えると実用的な意味があるわけではありません。（ただのロマンです。）

このミニゲームのコードは、以下のGitHubリポジトリで公開されています。興味のある方は、ぜひチェックしてみてください。
（リポジトリのリンクをここに挿入）

==　まとめ
この記事では、Jetpack Glanceを使ったウィジェット開発について解説しました。
Glanceを利用することで、Jetpack Composeの記法でウィジェットを作成することができ、開発者はより効率的にウィジェットを開発できます。
また、Jetpack Composeの多様な機能を活用することで、ウィジェットの操作性や見た目を向上させることができます。

これらの機能を活かして、自分だけのオリジナルウィジェットを開発してみてはいかがでしょうか。


== リファレンス
参考となるドキュメントやリンクの紹介です。

 * Android Developers: Glance
https://developer.android.com/jetpack/androidx/releases/glance

 * Android Developers: Create a simple widget
https://developer.android.com/develop/ui/views/appwidgets
GlanceはRemoteViewsをラップしているものなので、内部の仕組みとしては既存のウィジェットの仕組みが生きています。
従来のウィジェットのドキュメントも有用です。


 * android/user-interface-samples/AppWidget
https://github.com/android/user-interface-samples/tree/main/AppWidget
公式のウィジェットサンプルです。やはり公式のサンプルコードを見るのが一番です。