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
完全に


=== どうやってデータ自体を更新する？
androidx.glance.appwidget.state.GlanceAppWidgetStateKt#updateAppWidgetState というメソッドが用意されており、引数に`updateState: suspend (MutablePreferences) -> Unit`としてdataStoreが使えます。



ウィジェットは複数作成することができ、それぞれ識別するidが存在します。この更新処理も引数にidを取るため、
複数配置したウィジェットのうちそれぞれで別のデータを保持・表示することも可能です。


ウィジェット側でデータを読み込むコード
☆ここにコード

しかし、通常のJetpackComposeとは違い、GlanceではComposable関数の引数に渡したデータが更新されても勝手にRecomposeをしてくれるわけではありません。
手動でトリガーを引く必要があります。
GlanceAppWidgetにupdate関数が用意されているので、データを更新した後にそれをコールします。
//list[手動更新][手動でcomposeのトリガーを引く]{
GlanceAppWidgetSample().update(context, id)
//}

idを意識しない場合はupdateAll(context)も使えます。例えばタップされたウィジェットのみ更新するときはupdate(),
例えば天気情報などすべてのウィジェットに反映すべきデータの更新後はupdateAll()を使うなど使い分けることができます。


これらをまとめたReceiverでのonReceive()処理は以下の感じです。
//list[onReceive][onReceive処理]{
    override fun onReceive(context: Context, intent: Intent) {
        super.onReceive(context, intent)
        android.util.Log.e("呼ばれた", "onReceive入り口")

//        if (intent.action == ACTION_REQUEST_UPDATE) {
            android.util.Log.e("呼ばれた", "onReceive")

            update(context)
//        }
    }

    private fun update(context: Context) {
        android.util.Log.e("呼ばれた", "update")

        scope.launch {
            val range = (0..9999999999)
            val randomList = listOf<String>(
                range.random().toString(),
                range.random().toString(),
                range.random().toString(),
                range.random().toString(),
            )

            val colorRange = (0..100)
            val red = colorRange.random()
            val green = colorRange.random()
            val blue = colorRange.random()

            val ids =
                GlanceAppWidgetManager(context).getGlanceIds(GlanceAppWidgetSample::class.java)
            ids.forEach { id ->
                updateAppWidgetState(context, id) { pref ->
                    pref[stringPreferencesKey(GlanceAppWidgetSample.KEY_PREFERENCES_LIST)] =
                        randomList.joinToString()

                    pref[intPreferencesKey(GlanceAppWidgetSample.KEY_PREFERENCES_RED)] = red
                    pref[intPreferencesKey(GlanceAppWidgetSample.KEY_PREFERENCES_BLUE)] = blue
                    pref[intPreferencesKey(GlanceAppWidgetSample.KEY_PREFERENCES_GREEN)] = green
                }
                GlanceAppWidgetSample().update(context, id)
            }


//            AlarmManagerを使って定期的に更新する場合の処理。 ただし最短で5秒の制限があるため使えない
//            val alarmManager = context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
//            alarmManager.setExact(
//                AlarmManager.RTC_WAKEUP,
//                System.currentTimeMillis() + UPDATE_INTERVAL,
//                createUpdatePendingIntent(context)
//            )
        }
    }
//}


=== 何を使ってトリガーを引く？ ここに決定木みたいなの
手動更新のみでOK → クリックイベントの中で更新
一日1回でOK → onUpdate に 処理を書いてupdatePeriodMillis でOK？？
30分に1回 → WorkManagerが良さそう？ もしくは updatePeriodMillis
5分に1回 → AlarmManager
それより高頻度 BroadcastIntentを投げる
もしくは CoroutineのDelayで 自身の更新

高速更新は正直そもそもウィジェットに必要なさそう。
あるとしたら秒時計とか？
いずれの場合にも手動更新ボタンを用意しておくのが良さそうです。

==== 手動更新

==== xmlに書いた時間で更新する
updatePeriodMillis に指定する。でもこれは最低30分（要ソース）これを使うのが行儀がよいと思われる、けど実際にはもっと高頻度で更新したかったりする

==== 自分でupdateをかける

==== WorkManager
アプリの状態に依存せずに処理を継続できるという点で、これが一番行儀よくできそう。
公式のサンプルアプリもこれを使っている。

==== 
Receiverがいる。ということはBroadCastintentをなげまくってなんとかできる

=== 落とし穴 …でもないか、要件次第かな？？？
==== 落とし穴① AlarmManager
最短5秒

==== 落とし穴②
BroadcastIntentでも秒間10回までが限度？？？これはもっと研究するか


== ウィジェットの高度な機能

=== ウィジェットのサイズ変更を検知してレイアウト変更

widgetManager.getAppWidgetInfo でなんかSizeが取れそう？？

=== 複数のウィジェットの考慮



== その他Tips

onEnabled

unUpdate

onDisabled



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