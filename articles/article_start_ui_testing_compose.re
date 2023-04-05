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

==== ファインダー


ファインダーAPIを使うとツリーからノードを検索することができます。
いくつかのファインダーAPIが用意されており、例えば条件に一致するノードを検索するのはonNodeを利用します。onNode以降に繋げる操作は1つの要素が見つかったことを想定しています。
ノードが表示されていることを判定するテストを書くと次のようになります。

//list[onNode][onNode.kt]{
composeTestRule.onNode(hasText("Hello")).assertIsDisplayed()
//}

onNodeにはマッチャー（後述）を指定することができます。hasText("Hello")は「Hello」をtextに持っているノードを検索することができます。

要素が見つからなかった場合、複数の要素が見使った場合に次のようなAssertionErrorがスローされます。

//list[AssertionError][AssertionError]{
java.lang.AssertionError: Failed to perform isDisplayed check.
Reason: Expected exactly '1' node but could not find any node that satisfies: (Text + EditableText contains 'Hello' (ignoreCase: false))
//}

onNodeにはuseUnmergedTree引数にtrueを設定することでマージされていないノードツリーから検索することができます。この引数は例えばButtonのようなツリーでマージされる要素に対して有効です。


==== マッチャー

前述のhasText()はマッチャーの一種です。マッチャーはツリーから要素を探すマッチングを実施できます。

次の例はhasParentを利用して特定の親ノードを持つ要素を検索します。(@<list>{hasParent})

//list[Button][Button.kt]{
Button(modifier = Modifier.testTag("Button"), ...) {
    Text(text = "Submit")
}
//}

//list[hasParent][hasParent.kt]{
composeTestRule.onNode(hasParent(hasTestTag("Button")), useUnmergedTree = true)
//}



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

