= Glanceで遊ぼう

== Glanceって何さ

Glanceとは、AndroidのWidgetをJetpackComposeの書き方で書けるうんぬんかんぬん

=== そもそもWidgetってどんなのだっけ
Widgetとはうんぬんかんぬん。静的な情報とか RemoteViewとかの実装が必要で

=== JetpackComposeで書けるってどういうこと
記法が使えるよ、でもRecomposeとかはないよ Viewをつくってまるっと更新するイメージだよ


== 環境構築
Glanceの開発をはじめるためにまずはComposeの環境を整えるよ（@<list>{build.gradle})

//list[build.gradle][build.gradle]{
androidImplementation("")hogehoge compose
//}

Glanceの開発をはじめるためにはbuild.gradleに次の依存関係を追加します。（@<list>{build.gradle2})

//list[build.gradle2][build.gradle]{
androidImplementation("")hogehoge glance 
//}


== Glanceを利用したシンプルなウィジェットの構築
まずは画面更新などを考えない、シンプルなウィジェットを表示してみましょう。

Glanceでは GlanceAppWidgetReceiver と GlanceAppWidget という登場人物がいるよ
それぞれ継承したクラスを作っていくよ


→ あれ、とりあえず最初はReceiverいなくても動く気がしてきたので試す。


=== GlanceAppWidgetReceiverの作成

=== GlanceAppWidgetの作成
こいつが@Composableを持っている

=== meta-data.xmlの作成
更新頻度（後述）など

=== AndroidManifestに追記

これは通常のBroadcastReceiver追加時と同じだけど忘れずに

//list[AndroidManifest][AndroidManifest.xml]{
<receiver
    android:name=".GlanceAppWidgetReceiverSample"
    android:exported="false">
    <intent-filter>
        <action android:name="android.appwidget.action.APPWIDGET_UPDATE" />
        <action android:name="action_request_update" />
    </intent-filter>
    <meta-data
        android:name="android.appwidget.provider"
        android:resource="@xml/glance_appwidget_sample_meta_data" />
</receiver>
//}


=== ビルドしてみよう
ビルドしてウィジェットをホームに追加すると…

ここに画像


とりあえずGlanceハローワールド成功です！！



== ウィジェットの表示をカスタム

リスト表示とか。
サイズ変更とか。

=== Modifierがいない！？
いません。ちなみに間違えて使ってしまうと謎に丁寧なエラー表示に遭遇します。



== ウィジェットの表示を動的に更新する


=== どうやってデータ自体を更新する？
dataStoreが使える。


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



== その他Tips


== おまけ Glanceでミニゲーム

電池消費や画面描画の遅さ、またホーム画面でそれを行う意義などを考えるとまったく実用的ではないけどロマンです

=== 要件
これまでの総集編 メタ的な要素

これから開発