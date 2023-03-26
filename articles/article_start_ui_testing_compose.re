= Composeで始めるUIテスト

== コンポーザブルをテストする

=== 依存関係

コンポーザブルをテストするためにbuild.gradleに次の依存関係を追加します。（@<list>{build.gradle})

//list[build.gradle][build.gradle]{
androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.4.0")
}//

JUnit4(テストフレームワーク)にComposeを統合します。このモジュールにはテストで利用するComposeTestRuleとComposeContentTestRulegが含まれています。

=== UI階層からComposeを選択

==== マッチャーによる選択

=== アサーション確認

=== アクション

== セマンティクスツリーを理解する

=== マージされたツリーとマージされていないツリー

=== ツリーをデバッグする

==== printToLog()

== Activityとリソースにアクセスする

=== createAndroidComposeRule

== 状態の復元のテスト

=== StateRestorationTester

== Firebase Robo Test での認証

