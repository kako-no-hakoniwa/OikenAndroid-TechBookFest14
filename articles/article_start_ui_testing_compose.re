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

useUnmergedTree引数はマージされていないツリーからノードを検索する時にtrueを設定します。例えばボタンなどのコンポーザブル内を検索したい時に有効です。


=== アサーション確認

アサーションはプログラムが正常に動作しているかどうかを検証するために使用され、期待通りに動作していることを保証することができます。
Jetpack Composeではassert()を呼び出すことでアサーションを確認できます。(@<list>{assert})

//list[assert][assert.kt]{
composeTestRule
   .onNode(hasParent(hasTestTag("Button")), useUnmergedTree = true)
   .assert(hasText("Submit"))
//}

assert()内ではorやandで複数のマッチャーを用いることができます。(@<list>{multiMatcher})

//list[multiMatcher][multiMatcher.kt]{
composeTestRule
    .onNode(hasParent(hasTestTag("Button")), useUnmergedTree = true)
    .assert(hasText("Submit") or hasText("Cancel"))
//}

=== アクション

ノードに対してクリックやスワイプなどのアクションを実行したい場合は、performプリフィックスがついている関数を呼び出します。
たとえば、クリックを実行するにはperformClick()を呼び出します。(@<list>{performClick})

//list[performClick][performClick.kt]{
composeTestRule
    .onNode(hasParent(hasTestTag("Button")), useUnmergedTree = true)
    .performClick()
//}

perform関数の中でアクションを複数指定することはできません。ノードに対して複数アクションを行なう場合はperform関数を複数回呼び出します。
次のコードは、スクロールした後にクリックを実行するアクション例です。(@<list>{performClickAfterPerformScroll})

//list[performClickAfterPerformScroll][performClickAfterPerformScroll.kt]{
composeTestRule
    .onNode(hasParent(hasTestTag("Button")), useUnmergedTree = true)
    .performScrollTo()
    .performClick()
//}

== セマンティクスツリーを理解する

ここまでJetpack ComposeでUIテストを実行するための基本操作を紹介してきました。この節はJetpack ComposeがどのようにUI階層を理解するのか紹介していきます。

Jetpack Composeのセマンティクスツリーは、テストフレームワーク以外にユーザー補助サービスからの認識に利用されます。

セマンティクスツリーという時、2つのツリーをことを指します。子孫ノードをマージされたセマンティクスツリーとマージを適応せずすべてのノードをそのままにするマージされていないセマンティクスツリーがあります。

=== マージされたツリー

画面に表示されている内容を正しい意味で使えるには、子孫ノードを個別に扱うのではなくツリーをマージして1つのノードとして扱うと便利な場合があります。
ボタンなどの複数の子ノードを含むコンポーザブルは、マージされたツリーでは1つのノードとして扱われます。これはユーザー補助サービスを使用する場合のフォーカス可能な要素を表す意味で有効です。

Modifierにはセマンティクスプロパティをマージを指定することができるsemantics修飾子があります。(@<list>{semantics})

//list[semantics][semantics.kt]{
Modifier.semantics (mergeDescendants = true) {}
//}

semantics修飾子以外にもclickable修飾子とtoggleable修飾子を使用した場合にもその子孫ノードがマージされます。(@<list>{clickable})

//list[clickable][clickable.kt]{
Modifier.clickable { /*TODO*/ }
//}


=== マージされていないツリー

テスト時にツリーからノードを検索したい場合、マージされていないツリーを用いると便利です。
テスト中にツリーをデバッグした時printToLog()で確認できます。(@<list>{printToLog})

//list[printToLog][printToLog.kt]{
composeTestRule.onRoot().printToLog("TAG")
//}

printToLog()を呼び出すとLogcatに次のような出力が表示されます。(@<list>{Logcat})

//list[Logcat][Logcat.sh]{
 Node #1 at (l=0.0, t=108.0, r=243.0, b=234.0)px
    |-Node #3 at (l=0.0, t=119.0, r=243.0, b=224.0)px, Tag: 'Button'
    Role = 'Button'
    Focused = 'false'
    Text = '[Submit]'
    Actions = [OnClick, RequestFocus, GetTextLayoutResult]
    MergeDescendants = 'true'
//}

マージされていないツリーを使う必要がある時、useUnmergedTreeをtrueに設定します。(@<list>{useUnmergedTreeTrue})

//list[useUnmergedTreeTrue][useUnmergedTreeTrue.kt]{
composeTestRule.onRoot(useUnmergedTree = true).printToLog("TAG")
//}

== Activityとリソースにアクセスする

=== createAndroidComposeRule

== 状態の復元のテスト

=== StateRestorationTester

== Firebase Robo Test での認証
