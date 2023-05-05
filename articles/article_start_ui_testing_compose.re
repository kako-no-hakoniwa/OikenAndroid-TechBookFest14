= Composeで始めるUIテスト

UIテストは、ユーザーインターフェース（UI）を対象とした自動化テストのことです。UIテストは、ソフトウェアのUIが意図した通りに機能し、ユーザーが期待する動作をするかどうかを確認できます。
Jetpack composeで実装されたUIをテストする場合、UIテストを実行するためのAPIが提供されておりUIテストを簡単に実装することができます。この章では、Jetpack ComposeでUIテストを実装する方法を説明します。

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
Reason: Expected exactly '1' node but could not find any node that satisfies:
 (Text + EditableText contains 'Hello' (ignoreCase: false))
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

//list[LogcatUnMerged][LogcatUnMerged.sh]{
Node #1 at (l=0.0, t=108.0, r=243.0, b=234.0)px
    |-Node #3 at (l=0.0, t=119.0, r=243.0, b=224.0)px, Tag: 'Button'
    Role = 'Button'
    Focused = 'false'
    Actions = [OnClick, RequestFocus]
    MergeDescendants = 'true'
    |-Node #5 at (l=63.0, t=147.0, r=180.0, b=196.0)px
        Text = '[Submit]'
        Actions = [GetTextLayoutResult]
//}


== Activityとリソースにアクセスする

Activityのリソースにアクセスする必要がある時、createComposeRuleの代わりにcreateAndroidComposeRuleを利用します。
この時、空のActivityを使用してAndroidComposeTestRuleを作成します。@<list>{createAndroidComposeRule}の例はComponentActivityを作成してリソースにアクセスするテストです。

AndroidManifest.xmlにComponentActivityを追加する必要があります。そのために次の依存関係を追加します。(@<list>{manifestdependency})

//list[manifestdependency][manifest.gradle]{
debugImplementation("androidx.compose.ui:ui-test-manifest:1.4.0")
//}

//list[manifest][manifest.kt]{
<application
    //...
    <activity android:name=".ComponentActivity" />
</application>
//}

//list[createAndroidComposeRule][createAndroidComposeRule.kt]{
@get:Rule
val composeTestRule = createAndroidComposeRule<ComponentActivity>()

@Test
fun `AndroidComposeTestRuleのテスト`() {
    composeTestRule.setContent {
        MyAppTheme {
            HelloCompose()
        }
    }
    val label = composeTestRule.activity.getString(R.string.label)
    composeTestRule.onNodeWithText(label).assertIsDisplayed()
}
//}


== 状態の復元のテスト

アクティビティの再生成された場合、コンポーザブルの状態が正しく復元されているか検証する必要があります。
例えば、rememberSaveableを実装した次のようなコンポーザブルがあるとします。(@<list>{rememberSaveable})

//list[rememberSaveable][rememberSaveable.kt]{
@Composable
fun Counter() {
    var count by rememberSaveable {
        mutableStateOf(0)
    }
    Column {
        Text(
            text = "$count"
        )
        IconButton(onClick = { count++ }) {
            Icon(
                imageVector = Icons.Default.Add,
                contentDescription = "increment"
            )
        }
    }
}
//}

Counter()コンポーザブルはアクティビティが再生成された場合でもcountの状態を保持します。
このようなコンポーザブルをテストする時、StateRestorationTesterを利用することができます。(@<list>{stateRestorationTester})

//list[stateRestorationTester][stateRestorationTester.kt]{
@get:Rule
val composeTestRule = createComposeRule()
    
@Test
fun `状態の復元をチェックするテスト`() {
    val restorationTester = StateRestorationTester(composeTestRule)

    restorationTester.setContent {
        CounterScreen()
    }

    composeTestRule
        .onNode(hasText("0"))
        .assertIsDisplayed()

    composeTestRule
        .onNode(hasContentDescription("increment"))
        .performClick()

    restorationTester.emulateSavedInstanceStateRestore()

    composeTestRule
        .onNode(hasText("1"))
        .assertIsDisplayed()
}
//}

== ナビゲーションのテスト

実際のアプリは複数の画面を遷移（以下、ナビゲーション）させることが一般的です。
ナビゲーションの実装がされているアプリであれば、ナビゲーションのテストも行うことが望ましいです。

Composeでナビゲーションのテストを行う場合、次の依存関係を追加します。(@<list>{navigationDependency})

//list[navigationDependency][navigationDependency.gradle]{
androidTestImplementation "androidx.navigation:navigation-testing:2.5.3"
//}

TestNavHostControllerのインスタンス化します。@Beforeアノテーションをつけたメソッドにインスタンス化を行う実装を処理しておきます。(@<list>{before})

//list[before][before.kt]{
@get:Rule
val composeTestRule = createComposeRule()
private lateinit var navController: TestNavHostController

@Before
fun setUpNavHost() {
    composeTestRule.setContent {
        navController = TestNavHostController(LocalContext.current)
        navController.navigatorProvider.addNavigator(ComposeNavigator())
        AppNavHost(navController = navController)
    }
}
//}

以降、ナビゲーションのテストを実行できるようになります。
ログインボタンを押した後、想定される遷移した先のRouteのアサーションを確認するテストは次のようになります。(@<list>{navigationTest})

//list[navigationTest][navigationTest.kt]{
@Test
fun `ログイン後カウンター画面に遷移しているかチェックするテスト`() {
    composeTestRule
        .onNode(hasText("Login"))
        .performClick()

    val route = navController.currentBackStackEntry?.destination?.route

    assertEquals(route, "counter")
}
//}

== 運用に乗せる

=== CIでUIテストを自動化する

UIテストをPullRequest作成時やPush時に自動で実行することで、手動でテスト実行するコストの削減と追加実装によるデグレーションが起きていないかチェックを自動で行うことができます。
CIサービスを用いている環境であれば簡単な設定でUIテストを自動化することができます。

例えばCircleCIではAndroidシステムのOrbが用意されているため、次のような設定でCircleCI上でUIテストを実行できます。(@<list>{ciTest})

//list[ciTest][ciTest.yml]{
jobs:
  android-test:
    executor:
      name: android/android-machine
      resource-class: xlarge
    steps:
      - checkout
      - android/start-emulator-and-run-tests:
          test-command: ./gradlew connectedDebugAndroidTest
          system-image: system-images;android-30;google_apis;x86
//}

=== Firebase Test Lab を利用したUIテスト自動化

多種多様なデバイスや構成でテストを行いたい場合にFirebase Test Labを用いると便利です。
Firebase Test Labの利用はConsole上でも行えますが、gcloud CLIによりローカルPCやCIサービス上でも利用できます。

Firebase Test Labする前にテスト対象アプリとテスト実行アプリをビルドする必要があります。(@<list>{buildApk})

//list[buildApk][buildApk.sh]{
./gradlew :app:assembleDebug
./gradlew :app:assembleDebugAndroidTest
//}

デフォルトでは次のフォルダ内にapkファイルが生成されます。

//list[apkholder][apkholder.sh]{
/app/build/outputs/apk
//}

生成できたapkをFirebase Test Labでテスト実行します。
インストルメンテーションテストを実行する場合次のようなコマンドで実行することができます。

//list[gcloudtest][gcloudtest.sh]{
gcloud firebase test android run \
  --type instrumentation \
  --app app/build/outputs/apk/debug/app-debug.apk \
  --test app/build/outputs/apk/androidTest/debug/app-debug-androidTest.apk \
  --device model=panther,version=33,locale=en,orientation=portrait  \
  --client-details matrixLabel="Example matrix label"
//}

Firebaseコンソールで次のようにテスト結果を確認することができます。

//image[test_lab_result][Firebase Test Labのコンソール][scale=0.75]{

//}


=== Firebase Robo Test での認証
Firebase Test Labにはインストルメンテーションテストの他にRobo Testというテストツールがあります。
Robo Testはアプリのユーザー インターフェースを分析し、実行可能なアクティビティを自動的にシミュレートしてテストを実行します。

インストルメンテーションテストと異なりテストコードを必要としないのが特徴です。

アプリにログイン画面などの認証がある場合、Robo Testでは設定をしておくことで自動で認証を行うことができます。
認証はコンソールのGUI上で設定できる他に、Robo スクリプトを実行時に指定することができます。

Composeで実装されたログイン画面の認証を行う場合、Robo Testが任意のコンポーザブルにアクセスできるようにModifier.testTagを設定します。

//list[composeLogin][composeLogin.sh]{
@Composable
fun LoginScreen() {
    Column(
        // ...
    ) {
        OutlinedTextField(
            modifier = Modifier
                .testTag("emailLogin"),
            // ...
        )
        OutlinedTextField(
            modifier = Modifier
                .testTag("passwordLogin"),
            // ...
        )
        Button(
            modifier = Modifier
                .testTag("buttonLogin"),
            // ...
        ) {
            Text(text = "Login")
        }
    }
}
//}

このLoginScreenコンポーザブルをRobo Testで認証させる場合、例えば次のようなRobo スクリプトを渡すと認証を自動で行えます。

//list[roboScript][roboScript.sh]{
[
  {
    "crawlStage": "crawl",
    "contextDescriptor": {
      "condition": "app_under_test_shown"
    },
    "actions": [
      {
        "eventType": "VIEW_TEXT_CHANGED",
        "replacementText": "example@example.com",
        "elementDescriptors": [
          {
            "resourceId": "com.myapplication:id/emailLogin"
          }
        ]
      },
      {
        "eventType": "VIEW_TEXT_CHANGED",
        "replacementText": "password",
        "elementDescriptors": [
          {
            "resourceId": "com.myapplication:id/passwordLogin"
          }
        ]
      },
      {
        "eventType": "VIEW_CLICKED",
        "elementDescriptors": [
          {
            "resourceId": "com.myapplication:id/buttonLogin"
          }
        ]
      }
    }
  }
]
//}
