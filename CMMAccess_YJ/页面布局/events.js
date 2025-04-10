document.addEventListener("DOMContentLoaded", () => {
  var host = "";
  var offset = "0";
  var eventType = 0;
  var states = document.getElementById("states");
  var events = document.getElementById("events");
  var connect = function (event) {
    // update connection status
    var status = document.getElementById("connection-status");
    status.textContent = "正在连推送服务...";
    status.setAttribute("class", "critical");

    // create websocket connection. 
    var url = document.location.href;
    var pos = url.indexOf('?');
    if(-1 == pos) {
      pos = url.length;
    }
    url = url.substring(0, pos);
    var url = url.replace("http", "ws") + "?offset=" + offset + "&host=" + host;
    var ws = new WebSocket(url);
    ws.onmessage = function (msg) {
      var event = JSON.parse(msg.data);
      if(event.type == 'meterDataChanged' || event.type == 'meter') {
        var dev = document.getElementById(event.msg.devId);
        if(null == dev) {
          dev = document.createElement('tbody');
          dev.setAttribute("id", event.msg.devId);
          states.appendChild(dev);
        }
        var meterElementId = '' + event.msg.devId + '-' + event.msg.meterId;
        var meter = document.getElementById(meterElementId);
        if(null == meter) {
          meter = document.createElement("tr")
          meter.setAttribute("id", meterElementId);
          dev.appendChild(meter);
          // populate data
          var devId = document.createElement("td");
          meter.appendChild(devId);
          devId.textContent = event.msg.devId;
          var aliasDevId = document.createElement("td");
          meter.appendChild(aliasDevId);
          aliasDevId.textContent = event.msg.aliasDevId;
          var meterId = document.createElement("td");
          meter.appendChild(meterId);
          meterId.textContent = event.msg.meterId;
          var meterName = document.createElement("td");
          meter.appendChild(meterName);
          meterName.textContent = event.msg.meterName;
          var value = document.createElement("td");
          meter.appendChild(value);
          value.textContent = event.msg.val;
          var time = document.createElement("td");
          meter.appendChild(time);
          time.textContent = event.msg.time;
          var meterType = document.createElement("td");
          meter.appendChild(meterType);
          meterType.textContent = event.msg.meterType;
        } else {
          // populate data
          meter.childNodes[0].textContent = event.msg.devId;
          meter.childNodes[1].textContent = event.msg.aliasDevId;
          meter.childNodes[2].textContent = event.msg.meterId;
          meter.childNodes[3].textContent = event.msg.meterName;
          meter.childNodes[4].textContent = event.msg.val;
          meter.childNodes[5].textContent = event.msg.time;
          meter.childNodes[6].textContent = event.msg.meterType;
        }
      } else if(event.type == 'alarm') {
        var rowId = '' + event.msg.serialNO + '-' + event.msg.beginTime;
        var row = document.getElementById(rowId);
        if(null == row) {
          var alarm = document.createElement("tr");
          alarm.setAttribute("id", rowId);
          // populate data
          var serialNo = document.createElement("td");
          alarm.appendChild(serialNo);
          serialNo.textContent = event.msg.serialNO;
          var devId = document.createElement("td");
          alarm.appendChild(devId);
          devId.textContent = event.msg.devId;
          var meterId = document.createElement("td");
          alarm.appendChild(meterId);
          meterId.textContent = event.msg.meterId;
          var describe = document.createElement("td"); // meterName
          alarm.appendChild(describe);
          describe.textContent = event.msg.describe;
          var beginTime = document.createElement("td");
          alarm.appendChild(beginTime);
          beginTime.textContent = event.msg.beginTime;
          var endTime = document.createElement("td");
          alarm.appendChild(endTime);
          if(event.msg.alarmFlag == 'end') {
            endTime.textContent = event.msg.endTime;
          }
          var alarmLevel = document.createElement("td");
          alarm.appendChild(alarmLevel);
          alarmLevel.textContent = event.msg.alarmLevel;
          var alarmFlag = document.createElement("td");
          alarm.appendChild(alarmFlag);
          alarmFlag.textContent = event.msg.alarmFlag;
          if(event.msg.alarmFlag == 'end') {
            alarmFlag.textContent = '结束';
          } else {
            alarmFlag.textContent = '开始';
          }
          var triggerVal = document.createElement("td");
          alarm.appendChild(triggerVal);
          triggerVal.textContent = event.msg.triggerVal;
          var sendState = document.createElement("td");
          alarm.appendChild(sendState);
          if(event.msg.sendState == '1') {
            sendState.textContent = '发送成功';
          } else {
            sendState.textContent = '';
          }
          var sendTimes = document.createElement("td");
          alarm.appendChild(sendTimes);
          sendTimes.textContent = event.msg.sendTimes;
          var lastSendTime = document.createElement("td");
          alarm.appendChild(lastSendTime);
          lastSendTime.textContent = event.msg.lastSendTime;

          if(0 == events.childElementCount) {
            events.appendChild(alarm);
          } else {
            events.insertBefore(alarm, events.childNodes[0]);
          }
        } else {
          if(event.msg.alarmFlag == 'end') {
            row.childNodes[5].textContent = event.msg.endTime;
          }
          if(event.msg.alarmFlag == 'end') {
            row.childNodes[7].textContent = '结束';
          } else {
            row.childNodes[7].textContent = '开始';
          }
          if(event.msg.sendState == '1') {
            row.childNodes[9].textContent = '发送成功';
          } else {
            row.childNodes[9].textContent = '';
          }
          row.childNodes[10].textContent = event.msg.sendTimes;
          row.childNodes[11].textContent = event.msg.lastSendTime;
        }
      } else {
        // discard
        console.log(msg.data);
      }
      return;
    };

    ws.onopen = function (ev) {
      var status = document.getElementById("connection-status");
      status.textContent = "推送服务已连接。";
      status.setAttribute("class", "normal");
    }
    ws.onclose = connect;
  };

  connect(null);
});

