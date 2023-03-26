= Composeで始めるUIテスト

== コンポーザブルをテストする

=== 依存関係

コンポーザブルをテストするためにbuild.gradleに次の依存関係を追加します。（@<list>{build.gradle})

//list[build.gradle][build.gradle]{
androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.4.0")
//}

JUnit 4（テストフレームワーク）にComposeを統合します。このモジュールにはテストで利用するComposeTestRuleとComposeContentTestRulegが含まれています。

=== UI階層（Node）からComposeを選択する

ComposeのテストはUI階層（以下、Node）からコンポーザブル要素を選択して操作を行います。たとえば、「Hello」を表示するコンポーザブルを選択する場合テストコードは次のようになります。(@<list>{HelloComposeTest})

//list[HelloComposeTest][HelloComposeTest.kt]{
class HelloComposeTest {
    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun `Helloが表示されていることをチェックするテスト`() {
        composeTestRule.setContent {
            MyAppTheme {
                HelloCompose()
            }
        }
        composeTestRule.onNodeWithText("Hello").assertIsDisplayed()
    }
}
//}

createComposeRule()でComposeTestRuleを作成します。テスト対象のコンポーザブルをsetContentで設定します。
対象のコンポーザブルの中から要素を取得するためいくつかのファインダーAPIが用意されています。

==== ファインダーAPI

===== onNode

条件に一致するNodeを検索します。onNode以降に繋げる操作は1つの要素が見つかったことを想定しています。要素が見つからなかった場合、または複数の要素が見使った場合はSemanticsNodeInteraction.assertDoesNotExistがスローされます。

//list[onNode][onNode.kt]{
composeTestRule.onNode(hasText("Hello"))
//}


===== onNodeWithContentDescription
指定したcontentDescriptionを持つNodeを検索します。

//list[onNodeWithContentDescription][onNodeWithContentDescription.kt]{
composeTestRule.onNodeWithContentDescription("Welcome")
//}

==== マッチャーによる選択する

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

