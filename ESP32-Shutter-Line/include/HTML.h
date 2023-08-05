const char* WirelessControlTerminal = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>无线控制端</title>
</head>

<body>
    <center>
        <h1>选择一个模式</h1>

        <h2><a href="ManuallySetting.html">手动设置参数拍摄</a></h2>
        <h2><a href="Presets.html">使用预设参数拍摄</a></h2>
        <h2><a href="SetPresets.html">设置预设参数</a></h2>
        <h2><a href="Settings.html">其他设置</a></h2>

        <br>
        <p>ESP32-Shutter-Line</p>
        <p>Designed By SoTWild</p>
    </center>
</body>

</html>
)=====";

const char* ManuallySetting = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>参数设置</title>
</head>

<body>
    <center>
        <h1>请输入相应参数：</h1>
        <p>（时长单位为秒，快门速度小于一秒时正值，输入分母。大于一秒时负值，输入时长）</p>
        <p>（快门速度为0时自动使用机内设置）</p>
        <p>（无需自动对焦输入0，启用自动对焦输入1）</p>
        <form>
            <label for="input1">定时时长：</label>
            <input type="number" id="input1" name="input1"><br>
            <label for="input2">拍摄张数：</label>
            <input type="number" id="input2" name="input2"><br>
            <label for="input3">拍摄间隔：</label>
            <input type="number" id="input3" name="input3"><br>
            <label for="input4">自动对焦：</label>
            <input type="number" id="input4" name="input4"><br>
            <label for="input5">快门速度：</label>
            <input type="number" id="input5" name="input5"><br><br>
            <button type="button" onclick="openURL()">发送数据并开始拍摄</button>
        </form>
    </center>

    <script>
        function openURL() {
            var input1 = document.getElementById("input1").value;
            var input2 = document.getElementById("input2").value;
            var input3 = document.getElementById("input3").value;
            var input4 = document.getElementById("input4").value;
            var input5 = document.getElementById("input5").value;

            // 构建URL，将五个参数作为查询参数
            var url = '/M?input1=' + input1 + '&input2=' + input2 + '&input3=' + input3 + '&input4=' + input4 + '&input5=' + input5;

            // 使用window.location.href在原有窗口中打开URL
            window.location.href = url;
        }
    </script>
</body>

</html>
)=====";

const char* Countdown_1 = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>倒计时</title>
    <style>
        progress {
            width: 200px;
        }

        .hide {
            display: none;
        }
    </style>
</head>

<body>
    <center>
        <h1 id="countdownLabel">预计剩余时间: <span id="countdown"></span></h1>
        <div>
            <progress id="progressBar" value="0" max="100"></progress>
            <span id="progressText"></span>
        </div>
        <div id="endMessage" class="hide">
            <h1>拍摄结束</h1>
            <h1><a href="/">返回</a></h1>
        </div>
    </center>
    <script>
        function startCountdown(duration, displayLabel, display, progressBar, progressText, endMessage) {
            var timer = duration, hours, minutes, seconds;

            var countdownInterval = setInterval(function () {
                hours = parseInt(timer / 3600, 10);
                minutes = parseInt((timer % 3600) / 60, 10);
                seconds = parseInt(timer % 60, 10);

                hours = hours < 10 ? "0" + hours : hours;
                minutes = minutes < 10 ? "0" + minutes : minutes;
                seconds = seconds < 10 ? "0" + seconds : seconds;

                display.textContent = hours + ":" + minutes + ":" + seconds;

                progressBar.value = ((duration - timer) / duration) * 100;

                var percent = ((duration - timer) / duration) * 100;
                progressText.textContent = percent.toFixed(2) + "%";

                if (--timer < 0) {
                    clearInterval(countdownInterval);
                    displayLabel.classList.add("hide");
                    progressBar.style.display = "none";
                    progressText.style.display = "none";
                    endMessage.style.display = "block";
                }
            }, 1000);
        }

        window.onload = function () {
            var countdownTime =
)=====";
const char* Countdown_2 = R"=====(
,
                displayLabel = document.querySelector('#countdownLabel'),
                display = document.querySelector('#countdown'),
                progressBar = document.querySelector('#progressBar'),
                progressText = document.querySelector('#progressText'),
                endMessage = document.querySelector('#endMessage');

            startCountdown(countdownTime, displayLabel, display, progressBar, progressText, endMessage);
        };
    </script>
</body>

</html>
)=====";

const char* Presets_1 = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>使用预设参数拍摄</title>
</head>

<body>
    <center>
        <h1>选择一个预设：</h1>

        <h2>预设A：</h2>
        <p>定时时长：
)=====";
const char* Presets_2 = R"=====(
        <p>拍摄张数：
)=====";
const char* Presets_3 = R"=====(
        <p>拍摄间隔：
)=====";
const char* Presets_4 = R"=====(
        <p>自动对焦：
)=====";
const char* Presets_5 = R"=====(
        <p>快门速度：
)=====";
const char* Presets_6 = R"=====(
                    </p>
        <h3><a href="/PresetsA">使用预设A</a></h3>
        <br>

        <h2>预设B：</h2>
        <p>定时时长：
)=====";
const char* Presets_7 = R"=====(
                    </p>
        <h3><a href="/PresetsB">使用预设B</a></h3>
        <br>

        <h2>预设C：</h2>
        <p>定时时长：
)=====";
const char* Presets_8 = R"=====(
                    </p>
        <h3><a href="/PresetsC">使用预设C</a></h3>
    </center>
</body>

</html>
)=====";

const char* SetPresets_html = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>发送数据至服务器</title>
</head>

<body>
    <center>
        <h1>请输入相应参数进行设置：</h1>
        <p>（时长单位为秒，快门速度小于一秒时正值，输入分母。大于一秒时负值，输入时长）</p>
        <p>（快门速度为0时自动使用机内设置）</p>
        <p>（无需自动对焦输入0，启用自动对焦输入1）</p>
        <form>
            <h3>预设A：</h3>
            <label for="input1-1">定时时长：</label>
            <input type="number" id="input1-1" name="input1-1"><br>
            <label for="input2-1">拍摄张数：</label>
            <input type="number" id="input1-2" name="input1-2"><br>
            <label for="input3-1">拍摄间隔：</label>
            <input type="number" id="input1-3" name="input1-3"><br>
            <label for="input4-1">自动对焦：</label>
            <input type="number" id="input1-4" name="input1-4"><br>
            <label for="input5-1">快门速度：</label>
            <input type="number" id="input1-5" name="input1-5"><br><br>
            <button type="button" onclick="openURL('1')">设置</button>
            <hr>

            <h3>预设B</h3>
            <label for="input2-1">定时时长：</label>
            <input type="number" id="input2-1" name="input2-1"><br>
            <label for="input2-2">拍摄张数：</label>
            <input type="number" id="input2-2" name="input2-2"><br>
            <label for="input2-3">拍摄间隔：</label>
            <input type="number" id="input2-3" name="input2-3"><br>
            <label for="input2-4">自动对焦：</label>
            <input type="number" id="input2-4" name="input2-4"><br>
            <label for="input2-5">快门速度：</label>
            <input type="number" id="input2-5" name="input2-5"><br>
            <button type="button" onclick="openURL('2')">设置</button>
            <hr>

            <h3>预设C</h3>
            <label for="input3-1">定时时长：</label>
            <input type="number" id="input3-1" name="input3-1"><br>
            <label for="input3-2">拍摄张数：</label>
            <input type="number" id="input3-2" name="input3-2"><br>
            <label for="input3-3">拍摄间隔：</label>
            <input type="number" id="input3-3" name="input3-3"><br>
            <label for="input3-4">自动对焦：</label>
            <input type="number" id="input3-4" name="input3-4"><br>
            <label for="input3-5">快门速度：</label>
            <input type="number" id="input3-5" name="input3-5"><br>
            <button type="button" onclick="openURL('3')">设置</button>
            <hr>

            <h2><a href="/ClearPresets" style="color: red;">清空预设</a></h2>
        </form>

        <script>
            function openURL(group) {
                var input1 = document.getElementById("input" + group + "-1").value;
                var input2 = document.getElementById("input" + group + "-2").value;
                var input3 = document.getElementById("input" + group + "-3").value;
                var input4 = document.getElementById("input" + group + "-4").value;
                var input5 = document.getElementById("input" + group + "-5").value;

                // 构建URL，将参数作为查询参数，并添加额外参数group
                var url = '/SetPresets?group=' + group + '&input1=' + input1 + '&input2=' + input2 + '&input3=' + input3 + '&input4=' + input4 + '&input5=' + input5;

                // 使用window.location.href在原有窗口中打开URL 
                window.location.href = url;
            } </script>
    </center>
</body>

</html>
)=====";

const char* Success = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>成功设置</title>
</head>

<body>
    <center>
        <h1>设置成功！</h1>

        <h2><a href="/">返回主页</a></h2>
        <h2><a href="Presets.html">使用预设参数拍摄</a></h2>
    </center>
</body>

</html>
)=====";