= Glanceでウィジェットを作る

最近おいしい健康に入社したkumokumotです。この章ではAndroidのウィジェット開発を簡単にしてくれるJetpack Glanceについて説明します。

現在のおいしい健康Androidアプリではまだウィジェット機能は提供されていません。
しかしアプリ内の「買い物リスト」や「食事記録」など、ウィジェットとして提供することでアプリの使い勝手向上につながる機能がいくつかあり、個人的にもぜひウィジェットを導入したいと考えています。
今回はそんな思いのもとGlanceを試して得られた知識を共有します。


== Glanceとは
Glanceは、Androidデバイスのホーム画面上に配置できるウィジェットをJetpack Composeの記法を使って開発できるライブラリです。

従来のウィジェット開発では、@<code>{RemoteViews}@<fn>{source_remoteviews}を直接用いて実装する必要があり、またクリックイベントはすべて@<code>{Intent}を発行して制御する必要があるなど、開発が煩雑になる部分がありました。
Glanceでは、Composableを@<code>{RemoteViews}に変換してくれるため直接@<code>{RemoteViews}を意識する必要がなく、クリックイベントもコールバックの形で書けるなど、従来の作り方よりも簡単にウィジェットを作成できます。

現在（2023年5月現在）はベータ版@<fn>{source_glance_release}です。（5/10にアルファ版からベータ版になりました。）

=== Jetpack Composeの記法を使えるとはどういうことか
Glanceを使うことで、ウィジェット開発においてJetpack Composeの記法を活用できます。
ただし、「記法を利用できる」とは、Composable関数としての記述が可能であるという意味であり、通常のJetpack Composeと完全に同じ方法で記述できるわけではありません。

たとえば、一般的によく使われる@<code>{Modifier}はGlanceでは使用できず、その代わりに@<code>{GlanceModifier}が提供されています。
また、表示内容の差分のみ再コンポーズ@<fn>{source_recompose}してくれるような仕組みは存在せず、イベントに基づいて画面全体を更新するという考え方が適用されています。

しかしこれらの前提を理解した上で、すでにJetpack Composeを導入しているプロジェクトでは同様の記述方法でコーディングができるため、従来のウィジェット開発経験がなくても開発しやすいという点は大きなメリットです。

//footnote[source_remoteviews][https://developer.android.com/reference/android/widget/RemoteViews]
//footnote[source_glance_release][https://developer.android.com/jetpack/androidx/releases/glance#1.0.0-alpha05]
//footnote[source_recompose][https://developer.android.com/jetpack/compose/mental-model#recomposition]


== 環境構築
プロジェクトのbuild.gradleファイルにGlanceの依存関係を追加します。
//list[build.gradle][build.gradle]{
dependencies {
    implementation "androidx.glance:glance-appwidget:1.0.0-beta01"
}

android {
    buildFeatures {
        compose true
    }

    composeOptions {
        // 使用しているKotlinバージョンと互換性を持ったバージョンを指定 @<fn>{source_compose_kotlin}
        kotlinCompilerExtensionVersion = "1.4.5" 
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
}
//}

Glanceは専用のComposeを使うため、通常のJetpack Composeの依存関係の追加は不要です。

//footnote[source_compose_kotlin][https://developer.android.com/jetpack/androidx/releases/compose-kotlin]

== Glanceを利用したシンプルなウィジェットの構築：Hello World
まずは、画面更新などの要素を考慮せず、シンプルなウィジェットを用いてHello Worldを実現してみましょう。

Glanceでは、@<code>{GlanceAppWidget}と@<code>{GlanceAppWidgetReceiver}というクラスが主要な役割を担っています。
これらのクラスを継承したカスタムクラスを実装していくことで、ウィジェットを作成していきます。

=== GlanceAppWidgetの作成
まずはじめに、@<code>{GlanceAppWidget}クラスを継承したカスタムウィジェットクラスを作成しましょう。
@<code>{GlanceAppWidget}クラスには抽象メソッドとして@<code>{provideGlance(context: Context, id: GlanceId)}が定義されているためoverrideし、
その中で@<code>{provideContent(content: @Composable @GlanceComposable () -> Unit)}を呼び出しComposable関数を渡します。

//list[GlanceAppWidget][GlanceAppWidgetを継承してComposableを記述]{
import androidx.compose.runtime.Composable
import androidx.glance.appwidget.GlanceAppWidget
import androidx.glance.text.Text

class GlanceAppWidgetSample : GlanceAppWidget() {

    override suspend fun provideGlance(context: Context, id: GlanceId) {
        provideContent { Content() }
    }

    @Composable
    fun Content() {
        Text(text = "Hello, Glance!")
    }
}
//}
通常のComposeも導入しているプロジェクトでは、使用しているコンポーネントがGlance用のものであるかどうか注意が必要です。
今回の例では、Textコンポーネントが@<code>{androidx.glance.text.Text}であることを確認してください。

=== GlanceAppWidgetReceiverの作成
@<code>{GlanceAppWidgetReceiver}は、ウィジェットのアップデートイベントをはじめとした各種イベントを受け取るためのクラスであり、これは@<code>{BroadcastReceiver}を継承しています。
ウィジェットを表示させるためには、最低限次の記述が必要です。先ほど作成した@<code>{GlanceAppWidget}を継承したクラスを指定してください。

//list[GlanceAppWidgetReceiver][GlanceAppWidgetReceiverを継承しglanceAppWidgetをoverride]{
import androidx.glance.appwidget.GlanceAppWidget
import androidx.glance.appwidget.GlanceAppWidgetReceiver

class GlanceAppWidgetReceiverSample : GlanceAppWidgetReceiver() {

    override val glanceAppWidget: GlanceAppWidget
        get() = GlanceAppWidgetSample()
}
//}


=== メタデータファイルの作成
ウィジェットの設定や動作に関する情報を定義するために、XMLでメタデータファイルを作成する必要があります。
単にウィジェットを表示するだけであれば属性@<fn>{source_metadata}を指定せずとも問題ありませんが、ここでは基本的な要素の指定を行いましょう。

//list[metadata][widget_meta_data]{
<?xml version="1.0" encoding="utf-8"?>
<appwidget-provider xmlns:android="http://schemas.android.com/apk/res/android"
    // ウィジェット追加時のプレビュー画像
    android:previewImage="@mipmap/ic_launcher"  
    // ウィジェットデフォルト幅・高さ
    android:minWidth="80dp"   
    android:minHeight="80dp"
    // Android12以降でのデフォルトのグリッド幅・高さ
    android:targetCellWidth="2"
    android:targetCellHeight="2"
    // ウィジェット長押しでリサイズ可能な方向
    android:resizeMode="horizontal|vertical"
     />
//}

//footnote[source_metadata][属性の種類は公式ドキュメント参照。https://developer.android.com/develop/ui/views/appwidgets#AppWidgetProviderInfo]

=== AndroidManifestに追記

前述のとおり、@<code>{GlanceAppWidgetReceiver}は@<code>{BroadcastReceiver}を継承しているため、忘れずにAndoridManifestにReceiver登録をしましょう。
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
ビルドが完了したら、実機またはエミュレータでアプリを起動し、ホーム画面に作成したウィジェットを追加してみましょう。正しく設定されていれば、ウィジェットに"Hello, Glance!"というテキストが表示されるはずです。

//image[hello_glance][GlanceでHello World][scale=0.3]{

//}

== ウィジェットの表示をカスタムする方法
ここからはウィジェットの表示をカスタムする方法を説明します。

=== GlanceModifier
通常のJetpack Composeでは、UIの作成において@<code>{Modifier}を使って要素のレイアウトやスタイルを変更するのが一般的です。しかし、Glanceでは@<code>{Modifier}を直接使用できず、代わりに@<code>{GlanceModifier}を使用します。
基本的な使い方は@<code>{Modifier}と同じですが、利用できる属性に制限がある@<fn>{source_glance_modifier}ため、通常の@<code>{Modifier}と同じようなレイアウトは組めないことがあります。

他、注意すべき点をいくつか説明します。
通常のComposeでは@<code>{Row}は@<code>{horizontalArrangement}、@<code>{Column}は@<code>{verticalArrangement}を引数に取りますが、Glanceの@<code>{Row}と@<code>{Column}はhorizontalとverticalの引数として@<code>{Alignment}しか取りません。
そのため、@<code>{Arrangement.SpaceAround}や@<code>{Arrangement.SpaceBetween}などを指定して要素を配置することは現状できません。
また、@<code>{RowScope}で提供されている@<code>{GlanceModifier}には@<code>{weight()}がなく@<code>{defaultWeight()}しかないため、自由にweightを指定できません。

...と、いくつかの制約はありますが、それでも基本的なレイアウトとスタイリングは十分に実現できます。

//footnote[source_glance_modifier][https://developer.android.com/reference/kotlin/androidx/glance/GlanceModifier#extension-functions_1]

=== リスト表示
Glanceでも@<code>{LazyColumn}が使用できます。ただし、現時点では@<code>{LazyRow}は利用できないため注意が必要です。

=== 画像の表示
Glanceでも通常のComposeと同様に画像を表示できます。
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
@<code>{GlanceModifier.clickable}を使うことでクリック時の処理を指定できます。
@<code>{clickable}は@<code>{androidx.glance.action.Action}というinterfaceを引数に取ります。この@<code>{Action}を実装することで処理を記述できます。

==== Activityの起動
いくつかの@<code>{Action}実装はあらかじめ用意されています。たとえばActivityの起動には@<code>{actionStartActivity()}を使います。

//list[Activityの起動][Activityの起動]{
Box(
    modifier = GlanceModifier.clickable(actionStartActivity<MainActivity>())
    ) {
    }
//}

また、@<code>{Broadcast}の送信には@<code>{actionSendBroadcast()}を、@<code>{Service}の起動には@<code>{actionStartService()}を使用できます。

==== 独自Actionの起動
独自のクリックアクションを定義するには、行いたい処理を@<code>{ActionCallback}を継承したクラスに記述し、そのクラスを@<code>{actionRunCallback}に型パラメータとして渡すことで実現できます。
//list[独自のクリックアクション][独自のクリックアクションの実装]{
Box(
    modifier = GlanceModifier.clickable(actionRunCallback<SampleAction>())
    ) {
        ...
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

=== データの更新
表示するためのデータの更新には、DataStore@<fn>{source_datastore}の仕組みが使われます。
DataStore自体にはPreferences DataStoreとProto　DataStoreの2種類がありますが、そのどちらもGlanceで利用できます。
具体的には、@<code>{androidx.glance.appwidget.state.GlanceAppWidgetStateKt#updateAppWidgetState()}というメソッドが引数違いで@<code>{Preferences DataStore}版と@<code>{Proto　DataStore}版のそれぞれ用意されています。

//footnote[source_datastore][https://developer.android.com/topic/libraries/architecture/datastore]

==== Preferences DataStoreを使ってウィジェットの状態を更新する
@<code>{updateAppWidgetState}（Preferences DataStore版）は次の引数を取ります。
//list[updateAppWidgetState(Preferences DataStore版)][updateAppWidgetState（Preferences DataStore版）]{
suspend fun updateAppWidgetState(
    context: Context,
    glanceId: GlanceId,
    updateState: suspend (MutablePreferences) -> Unit,
) {
    ...
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

セットしたデータはCompositionLocal@<fn>{source_composition_local}から取得できます。
//list[Preferences DataStoreを使ったデータの取得][Preferences DataStoreを使ったデータの取得]{
val prefs = currentState<Preferences>()
val string: String = prefs[stringPreferencesKey("key_name")]
//}

//footnote[source_composition_local][https://developer.android.com/jetpack/compose/compositionlocal]

==== Proto DataStoreを使ってウィジェットの状態を更新する
@<code>{updateAppWidgetState}（Proto DataStore版）は次の引数を取ります。

//list[updateAppWidgetState(Proto DataStore版)][updateAppWidgetState（Proto DataStore版）]{
suspend fun <T> updateAppWidgetState(
    context: Context,
    definition: GlanceStateDefinition<T>,
    glanceId: GlanceId,
    updateState: suspend (T) -> T,
): T {
    ...
}
//}
@<code>{GlanceStateDefinition<T>}は、型パラメータに保存するデータ型を取るinterfaceです。
次のように@<code>{GlanceStateDefitinition}を実装したクラスを作成します。

//list[GlanceStateDefitinitionの実装][GlanceStateDefitinitionの実装]{
object GlanceAppSampleStateDefinition : GlanceStateDefinition<SampleState> {

    private const val DATA_STORE_FILENAME = "sampleState"
    private val Context.datastore
            by dataStore(DATA_STORE_FILENAME, SampleStateSerializer)

    override suspend fun getDataStore(context: Context, fileKey: String)
            : DataStore<SampleState> {
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

Proto DataStoreを使って、先ほど定義した@<code>{SampleState.Success}状態にデータ更新を行うシンプルなサンプルコードは次のとおりです。
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
            updateState = { 
                SampleState.Success(displayString = "セットしたい文字列") 
            }
        )
    }
    GlanceAppWidgetSample().updateAll(context)
}
//}

セットしたデータを取り出すには次のように@<code>{stateDefinition}をoverrideし先ほどの@<code>{GlanceStateDefitinition}を実装したクラスを指定することでCompositonLocalから取得できます。
//list[Proto DataStoreを使ったデータの取得][Proto DataStoreを使ったデータの取得]{
class GlanceAppWidgetSample : GlanceAppWidget() {

    override val stateDefinition = GlanceAppSampleStateDefinition
    @Composable
    fun Content() {
        val sampleState = currentState<SampleState>()
        ...
    }
}
//}

=== ウィジェットの更新
通常のJetpack Composeでは、引数にとったデータの変更を検知し再コンポーズされますが、Glanceでは手動で画面の更新を呼び出す必要があります。
次のようにGlanceAppWidgetを継承したクラスの@<code>{update()}を呼び出します。
//list[手動更新update(context, id)][update(context, id)]{
GlanceAppWidgetSample().update(context, id)
//}

引数の@<code>{id}はウィジェットの識別idです。ウィジェットはホーム画面上に複数作成でき、それぞれにidが存在します。
たとえば前述のActionCallBackの@<code>{onAction()}には引数としてidが渡ってくるため、そのidを使ってタップされたウィジェットのみ更新できます。
idを意識しない場合は@<code>{updateAll(context)}も使えます。すべてのウィジェットに反映すべきデータの更新ではこちらを使います。

//list[手動更新updateAll(context)][updateAll(context)]{
GlanceAppWidgetSample().updateAll(context)
//}

=== 更新トリガー
これまでデータの更新方法を説明してきました。次に、更新処理をトリガーする方法について見ていきましょう。

前述のとおり、ウィジェットの構築にあたりwidget_meta_data.xmlというメタデータファイルを作成しました。
その中にある属性のひとつである@<code>{updatePeriodMillis}を使用して、ウィジェットの更新周期を指定できます。
指定した周期で@<code>{GlanceAppWidgetReceiver}の@<code>{onUpdate()}が呼び出されるため、更新処理を@<code>{onUpdate()}内に記述することで定期的な更新が実現できます。
ただし、@<code>{updatePeriodMillis}の最小値は1800000（=30分）に制限されており、より高頻度のデータ更新が必要な場合は、WorkManager@<fn>{source_workmanager}やAlarmManager@<fn>{source_alarmmanager}を使用して周期的な処理を実装する必要があります。

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

//footnote[source_workmanager][https://developer.android.com/topic/libraries/architecture/workmanager]
//footnote[source_alarmmanager][https://developer.android.com/training/scheduling/alarms]

===[column] onUpdate(), onEnabled(), onDisabled()

updatePeriodMillisで指定した周期でGlanceAppWidgetReceiverのonUpdate()が呼ばれると説明しましたが、他にもウィジェットの状態に応じて呼ばれるメソッドの代表を紹介します。

===== onUpdate()
ウィジェットがホーム画面に追加されたとき、updatePeriodMillisで指定した周期

===== onEnabled()
ウィジェットがホーム画面に追加されたとき

===== onDisabled()
ウィジェットがホーム画面から削除されたとき

これらはGlance特有のものではなく、従来のウィジェット実装と同様のため、既存のドキュメントを参照するのが正確です。
@<href>{https://developer.android.com/guide/topics/appwidgets?hl=ja#AppWidgetProvider}


===[/column]

== ウィジェットのサイズに応じたレイアウト変更
レスポンシブなレイアウトであればウィジェットのサイズが変わっても動的に対応できますが、それでは適切な表示を維持できない場合や、表示する要素数を変えたい場合があります。
ここではウィジェットのサイズによって表示するレイアウトごと（Composable関数ごと）変更する方法を説明します。

次のように、@<code>{GlanceAppWidget}を継承したクラスの@<code>{sizeMode}をoverrideし、@<code>{DpSize}のセットを指定することで、CompositionLocalから現在のサイズを取得できます。
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
    fun Content() {
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

==　まとめ
Glanceを使ったウィジェット開発について解説しました。
今までウィジェットを作ったことがない人でも、これまでに説明してきた内容で基本的なウィジェットは問題なく作成できる気がしてきたのではないでしょうか。

ぜひGlanceでウィジェットを作成して、アプリをより魅力的なものにしましょう。

=== リファレンス
参考となるドキュメントやリンクの紹介です。

 * Android Developers: Glance@<br>{}@<href>{https://developer.android.com/jetpack/androidx/releases/glance}

 * GitHub: android/user-interface-samples/AppWidget@<br>{}@<href>{https://github.com/android/user-interface-samples/tree/main/AppWidget}@<br>{}公式のウィジェットサンプルです。

 * Android Developers: Create a simple widget@<br>{}@<href>{https://developer.android.com/develop/ui/views/appwidgets}@<br>{}GlanceはRemoteViewsをラップしているものなので、内部の仕組みとしては既存のウィジェットの仕組みが動作しています。そのため従来のウィジェットのドキュメントも有用です。

===[column] おまけ Glanceミニゲーム

本章の内容を元に、ウィジェットでミニゲーム@<fn>{minigame}を作成してみました。

前方から次々に降ってくるAndroidロボット@<fn>{source_androidrobot}を集めるゲームです。

//image[glance_game][Glanceでミニゲーム（開発中）][scale=0.3]{

//}


コードは次のGitHubリポジトリで公開されています。興味のある方は、ぜひチェックしてみてください。
@<href>{https://github.com/kumokumot/GlanceGalaxyDroid}

//footnote[minigame][ウィジェットでミニゲームを作成すること自体は、バッテリー消費の問題や画面描画の遅さ、またホーム画面でそれを行う意義などを考えると実用的な意味があるわけではありません。（ただのロマンです。）]
//footnote[source_androidrobot][https://developer.android.com/distribute/marketing-tools/brand-guidelines#android_robot]

===[/column]