static const char index_html[] = R"EOF(
<!DOCTYPE html>
<html><head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 TIMELAPSE</title>
<style>
  body {
    font-family: Arial,Helvetica,sans-serif;
    background: #181818;
    color: #EFEFEF;
    font-size: 16px
  }

  section.main {
    display: flex
  }

  #menu,section.main {
    flex-direction: column
  }

  #menu {
    display: none;
    flex-wrap: nowrap;
    min-width: 340px;
    background: #363636;
    padding: 8px;
    border-radius: 4px;
    margin-top: -10px;
    margin-right: 10px;
  }

  #content {
    display: flex;
    flex-wrap: wrap;
    align-items: stretch
  }

  figure {
    padding: 0px;
    margin: 0;
  }

  figure img {
    display: block;
    width: 100%;
    height: auto;
    border-radius: 4px;
    margin-top: 8px;
  }

  section#buttons {
    display: flex;
    flex-wrap: nowrap;
    justify-content: space-between
  }

  #nav-toggle {
    cursor: pointer;
    display: block
  }

  #nav-toggle-cb {
    outline: 0;
    opacity: 0;
    width: 0;
    height: 0
  }

  #nav-toggle-cb:checked+#menu {
    display: flex
  }

  .input-group {
    display: flex;
    flex-wrap: nowrap;
    line-height: 22px;
    margin: 5px 0
  }

  .input-group>label {
    display: inline-block;
    padding-right: 10px;
    min-width: 47%
  }

  .input-group input,.input-group select {
    flex-grow: 1
  }

  .range-max,.range-min {
    display: inline-block;
    padding: 0 5px
  }

  button, .button {
    display: block;
    margin: 5px;
    padding: 0 12px;
    border: 0;
    line-height: 28px;
    cursor: pointer;
    color: #fff;
    background: #ff3034;
    border-radius: 5px;
    font-size: 16px;
    outline: 0
  }

  .switch {
    display: block;
    position: relative;
    line-height: 22px;
    font-size: 16px;
    height: 22px
  }

  .switch input {
    outline: 0;
    opacity: 0;
    width: 0;
    height: 0
  }

  .slider {
    width: 50px;
    height: 22px;
    border-radius: 22px;
    cursor: pointer;
    background-color: grey
  }

  .slider,.slider:before {
    display: inline-block;
    transition: .4s
  }

  .slider:before {
    position: relative;
    content: "";
    border-radius: 50%;
    height: 16px;
    width: 16px;
    left: 4px;
    top: 3px;
    background-color: #fff
  }

  input:checked+.slider {
    background-color: #ff3034
  }

  input:checked+.slider:before {
    -webkit-transform: translateX(26px);
    transform: translateX(26px)
  }

  select {
    border: 1px solid #363636;
    font-size: 14px;
    height: 22px;
    outline: 0;
    border-radius: 5px
  }

  .image-container {
    position: relative;
    min-width: 160px
  }
</style>
</head>
<body>
<section class="main">
  <div id="logo">
    <label for="nav-toggle-cb" id="nav-toggle">☰&nbsp;&nbsp;Toggle Timelapse settings</label>
  </div>
  <div id="content">
    <div id="sidebar">
      <input type="checkbox" id="nav-toggle-cb" checked="checked">
      <nav id="menu">
        <div class="input-group" id="framesize-group">
          <label for="framesize">撮影間隔</label>
          <select id="interval" class="default-action">
            <option value="1" selected="selected">1秒 (300枚で5分)</option>
            <option value="2">2秒 (300枚で10分)</option>
            <option value="5">5秒 (300枚で25分)</option>
            <option value="10">10秒 (300枚で50分)</option>
            <option value="30">30秒 (300枚で150分)</option>
          </select>
        </div>
        <div class="input-group" id="vflip-group">
          <label for="vflip">上下反転</label>
          <div class="switch">
            <input id="vflip" type="checkbox" class="default-action">
            <label class="slider" for="vflip"></label>
          </div>
        </div>
        <section id="buttons">
          <button id="toggle-stream">撮影開始</button>
        </section>
      </nav>
    </div>
    <figure>
      <img id="stream" src="" class="image-container">
    </figure>
  </div>
</section>
<script>
document.addEventListener('DOMContentLoaded', function (event) {
  const baseHost = document.location.origin
  const view = document.getElementById('stream')
  const streamButton = document.getElementById('toggle-stream')
  
  // サーバーに制御用のリクエストを出す
  updateConfig = (el) => {
    let value
    switch (el.type) {
      case 'checkbox':
        value = el.checked ? 1 : 0
        break
      case 'select-one':
        value = el.value
        break
      case 'button':
        value = '1'
        break
      default:
        return
    }
    fetch(`${baseHost}/control?var=${el.id}&val=${value}`)
  }

  // 映像配信停止
  stopStream = () => {
    window.stop();
    streamButton.innerHTML = '撮影停止'
    fetch(`${baseHost}/control?var=capture&val=1`) // 撮影開始
  }

  // 映像配信開始
  startStream = () => {
    fetch(`${baseHost}/control?var=capture&val=0`) // 撮影停止
    view.src = `${baseHost}:81/stream`
    streamButton.innerHTML = '撮影開始'
  }

  // 撮影ボタンが押されたら、映像配信停止と映像配信開始をトグルする
  streamButton.onclick = () => {
    const streamEnabled = streamButton.innerHTML === '撮影開始'
    if (streamEnabled) {
      stopStream()
    } else {
      startStream()
    }
  }

  // 'default-action'クラスの入力が変化したら、サーバーにリクエストを出す
  document
    .querySelectorAll('.default-action')
    .forEach(el => {
      el.onchange = () => updateConfig(el)
    })

  startStream() // 画面表示時に映像配信開始
})
</script>
</body></html>
)EOF";
