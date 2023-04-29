= Glanceで遊ぼう

== Glanceって何さ

Glanceとは、AndroidのWidgetをJetpackComposeの書き方で書けるうんぬんかんぬん
まだアルファなのでうんたらかんたら

=== そもそもWidgetってどんなのだっけ
Widgetとはうんぬんかんぬん。静的な情報とか RemoteViewとかの実装が必要で

=== JetpackComposeで書けるってどういうこと
Glanceを使うと、Androidのウィジェット開発において、Jetpack Composeの記法を利用してWidgetを記述できます。

ただし、Jetpack Composeの全ての機能がウィジェット開発で利用できるわけではありません。
例えば普段馴染みのあるModifierはGlanceでは使えません。代わりにGlanceModifierというものを使うよ

また、表示する内容を変更する際に、状態管理に関連するRecomposeのような仕組みではなくView全体を更新するイメージで進めることができます。

Jetpack Composeを使ったウィジェット開発では、従来のRemoteViewsを使った方法と比べてシンプルにウィジェットを作成できます。
またアプリ本体のコードにJetpack Composeを利用している場合、一貫性のあるコーディングスタイルを保つことが可能になります。


== 環境構築
☆念の為JetpackCompose自体の依存も書こうかな。 動作している環境は書くべきか。

Glanceを使用したウィジェット開発を始めるためには、まずJetpack Composeの開発環境を整える必要があります。この章では、環境構築に必要な手順を解説します。
プロジェクトのbuild.gradleファイルにJetpack ComposeとGlanceの依存関係を追加します。以下のコードをプロジェクトのbuild.gradleに追記します。（@<list>{build.gradle})

//list[build.gradle][build.gradle]{
dependencies {
    // 他の依存関係は省略
    implementation 'androidx.compose.ui:ui:<compose_version>'
    implementation 'androidx.compose.material:material:<compose_version>'
    implementation 'androidx.compose.ui:ui-tooling:<compose_version>'
    implementation 'androidx.glance:glance:<glance_version>'
    implementation 'androidx.glance:glance-compose:<glance_version>'
}
//}

Jetpack Composeを利用するためには、Android Gradle Pluginのバージョンも最新にする必要があります。プロジェクトのbuild.gradleに以下のように記述しましょう。（@<list>{build.gradle2})

//list[build.gradle2][build.gradle]{
buildscript {
    dependencies {
        // 他の依存関係は省略
        classpath 'com.android.tools.build:gradle:<agp_version>'
    }
}
//}

Jetpack Composeを利用するアプリケーションでは、Kotlinのバージョンも最新にする必要があります。プロジェクトのbuild.gradleに以下のように記述しましょう。（@<list>{build.gradle3})
//list[build.gradle3][build.gradle]{
buildscript {
    ext.kotlin_version = '<kotlin_version>'
    dependencies {
        // 他の依存関係は省略
        classpath 'org.jetbrains.kotlin:kotlin-gradle-plugin:$kotlin_version'
    }
}
//}

これで、Jetpack ComposeとGlanceを利用するための基本的な環境構築が完了しました。次の章では、実際にGlanceを利用したウィジェットの構築方法を解説します。

== Glanceを利用したシンプルなウィジェットの構築 Hello World
☆あれ、とりあえず最初はReceiverいなくても動く気がしてきたので試す。

まずは画面更新などを考えない、シンプルなウィジェットを表示してみましょう。

Glanceでは主に画面自体であるGlanceAppWidgetと、ウィジェット操作の起点になるGlanceAppWidgetReceiverが主なクラスです。
これらを継承したクラスを実装していきます。

=== GlanceAppWidgetの作成
まずは、GlanceAppWidgetクラスを継承したカスタムウィジェットクラスを作成します。このクラスでは、@Composableアノテーションを持った関数を使ってウィジェットのUIを記述します。

//list[GlanceAppWidget][GlanceAppWidgetを継承してComposableを記述]{
class SimpleGlanceAppWidget : GlanceAppWidget() {
    @Composable
    override fun Content() {
        Text(text = "Hello, Glance!")
    }
}
//}


=== GlanceAppWidgetReceiverの作成
☆ これってどこの処理で呼び出されてるか調べないとね


GlanceAppWidgetReceiverはウィジェットのアップデートイベントや、その他のウィジェットに関連するイベントを受け取るためのクラスです。
ウィジェットのアップデートなどは後述しますが、ひとまずウィジェットを表示させるためには最低限以下の記述が必要です、
//list[GlanceAppWidgetReceiver][GlanceAppWidgetReceiverを継承しglanceAppWidgetをoverride]{
class SimpleGlanceAppWidgetReceiver : GlanceAppWidgetReceiver() {
    override val glanceAppWidget: GlanceAppWidget
        get() = SimpleGlanceAppWidget()
}
//}

=== メタデータファイルの作成
☆どれが最低限マストなんだろう？？initialLayoutって必要？？実際には使われないけどまだ必要的な？


ウィジェットの設定や動作に関する情報を定義するためにXMLでメタデータファイルを作成する必要があります。
//list[metadata][simple_glance_appwidget_metadata]{
<appwidget-provider xmlns:android="http://schemas.android.com/apk/res/android"
    android:initialLayout="@layout/simple_glance_appwidget"
    android:minHeight="40dp"
    android:minWidth="40dp"
    android:updatePeriodMillis="1800000"
    android:widgetCategory="home_screen">
</appwidget-provider>
//}


=== AndroidManifestに追記

GlanceAppWidgetReceiverはBroadcastReceiverを継承しているため、通常の追加時と同じように忘れずにAndoridManifestに記述します。
また先程作成したメタデータファイルもManifestに記述します。

//list[AndroidManifest][AndroidManifest.xml]{
<receiver
    android:name=".SimpleGlanceAppWidgetReceiver"
    android:exported="false">
    <intent-filter>
        <action android:name="android.appwidget.action.APPWIDGET_UPDATE" />
    </intent-filter>
    <meta-data
        android:name="android.appwidget.provider"
        android:resource="@xml/simple_glance_appwidget_metadata" />
</receiver>
//}



☆metadata の中身をふるふるで書いたリストが欲しいな。
=== ビルドしてみよう
ビルドが完了したら、実機またはエミュレーターでアプリを起動し、ホーム画面に作成したウィジェットを追加してみましょう。正しく設定されていれば、ウィジェットに"Hello, Glance!"というテキストが表示されるはずです。

ここに画像
Hello World!!

これで、Glanceを利用してシンプルなウィジェットを構築する方法がわかりました。次の章では、ウィジェットの表示をカスタマイズしたり、動的に更新する方法などを解説します。


== ウィジェットの表示をカスタム
Hello Worldが成功したため、ここからは少しだけ表示をカスタムしていきましょう。

=== Modifierがいない！？
通常のJetpack ComposeでUIを作成する際、Modifierを使用して要素のレイアウトやスタイルを変更することが一般的です。
ただし、GlanceではModifierは直接利用できず、代わりにGlanceModifierというものを使います。基本的な使い方はModifierと同じなのですが、用意されている属性に限りがあるため、通常のModifierと同じようにレイアウトを組めない場合があります。

☆ここにGlanceで用意されている要素のリスト？？ もしくはないものの代表例


ここでコラム的な枠
☆間違ってModifierを使ったりするとこうなる
エラーの画像
Modifierを誤って使ってしまった場合、エラーメッセージが表示されることがあります。このような場合は、Glanceで利用できるコンポーズ要素や属性について調べ、適切な方法で表示をカスタマイズするようにしましょう。

=== リスト表示
LazyColumnが使えます。（ただしパッケージはandroidx.glance）

=== 画像の表示
普通の感じで書けます（ただし使うものはandroidx.glance.ImageKtです）
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
GlanceModifier.clickable を使う

=== Activityの起動
actionStartActivity


== ウィジェットの表示を動的に更新する
完全に


=== どうやってデータ自体を更新する？
androidx.glance.appwidget.state.GlanceAppWidgetStateKt#updateAppWidgetState というメソッドが用意されており、引数に`updateState: suspend (MutablePreferences) -> Unit`としてdataStoreが使えます。
glanceIdも引数に


ウィジェットは複数作成することができ、それぞれ識別するidが存在します。この更新処理も引数にidを取るため、
複数配置したウィジェットのうちそれぞれで別のデータを保持・表示することが可能です。


ウィジェット側でデータを読み込むコード
☆ここにコード

しかし、通常のJetpackComposeとは違い、GlanceではComposable関数の引数に渡したデータが更新されても勝手にRecomposeをしてくれるわけではありません。
手動でトリガーを引く必要があります。
GlanceAppWidgetにupdate関数が用意されているので、データを更新した後にそれをコールします。
//list[手動更新][手動でcomposeのトリガーを引く]{
GlanceAppWidgetSample().update(context, id)
//}



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



=== ここに決定木みたいなの

=== 何を使ってトリガーを引く？


==== xmlに書いた時間で更新する
updatePeriodMillis に指定する。でもこれは最低30分（要ソース）これを使うのが行儀がよいと思われる、けど実際にはもっと高頻度で更新したかったりする

==== クリックイベントで更新する
手動更新で十分であればupdatePeriodMillis と これの組み合わせでOK


==== 自分でupdateをかける
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
ウィジェット上で動作する簡単なミニゲームを作成するため、以下のような構造を考えます。

自機の移動
敵の生成と移動
衝突判定
スコアの表示
===　自機の移動
コントロールボタンを設置し、ボタンをクリックしたときに自機の位置を更新することで、自機の移動を実現します。

===　敵の生成と移動
一定時間ごとに、画面上部からランダムな位置で敵を生成し、敵を下方向に移動させることでゲームの難易度を作ります。

===　衝突判定
自機と敵が衝突した場合、ゲームオーバーとなります。衝突判定は矩形同士の判定を行うことで実現できます。

===　スコアの表示
ウィジェット上にスコアを表示し、敵を避けた回数に応じてスコアが増加する仕組みを実装します。

===　ゲームの最適化
ウィジェット上で動作するゲームのため、電池消費や画面描画の遅さを考慮し、最適化を行います。

===　ゲームの完成
ここまでで、Glanceを使ったミニゲームが完成しました。ウィジェット上で遊べるシンプルなゲームが実現できました。

===　まとめ
この記事では、Jetpack ComposeのGlanceを使ったウィジェット開発について紹介しました。Glanceを使えば、Jetpack Composeの記法でウィジェットを作成できるため、開発者にとってより効率的な開発が可能です。




=== 要件
これまでの総集編 メタ的な要素

これから開発