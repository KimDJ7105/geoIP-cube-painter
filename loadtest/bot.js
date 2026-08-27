// GeoIP Cube Painter 부하 테스트 / NPC 데모 스크립트
//
// 각 VU(Virtual User)가 하나의 "봇" 플레이어 역할을 합니다.
// 접속 후 무작위 방향(WASD)을 주기적으로 바꿔가며 실제로 맵을 돌아다니고 칠하는
// 살아있는 트래픽을 만듭니다 (단순 접속 유지가 아니라 실제 게임 로직을 태우는 부하).
//
// 사용 예:
//   로컬 소규모 리허설:
//     k6 run -e TARGET_HOST=127.0.0.1 -e VUS=10 -e DURATION=30s loadtest/bot.js
//   배포 후 데모용(적은 봇 수, 영상 녹화용):
//     k6 run -e TARGET_HOST=<EC2 퍼블릭 IP> -e VUS=30 -e DURATION=180s loadtest/bot.js
//   배포 후 대규모 스트레스 테스트:
//     k6 run -e TARGET_HOST=<EC2 퍼블릭 IP> -e VUS=1000 -e DURATION=60s loadtest/bot.js

import ws from 'k6/ws';
import { check } from 'k6';

const TARGET_HOST = __ENV.TARGET_HOST || '127.0.0.1';
const TARGET_PORT = __ENV.TARGET_PORT || '7777';
const MOVE_INTERVAL_MS = parseInt(__ENV.MOVE_INTERVAL_MS || '800', 10);

export const options = {
  vus: parseInt(__ENV.VUS || '10', 10),
  duration: __ENV.DURATION || '30s',
  // 봇은 스스로 연결을 끊지 않고 계속 열어두는 구조라, 기본 30초 유예(gracefulStop)를 그대로 두면
  // 매번 DURATION + 30s만큼 걸림. 정확한 시간에 종료되도록 꺼둠.
  gracefulStop: '0s',
};

const PacketType = { KEY_INPUT: 1 };
const KEYS = ['W', 'A', 'S', 'D'];

function buildKeyInputPacket(keyChar, isDown) {
  const buf = new ArrayBuffer(3);
  const view = new DataView(buf);
  view.setUint8(0, PacketType.KEY_INPUT);
  view.setUint8(1, keyChar.charCodeAt(0));
  view.setUint8(2, isDown ? 1 : 0);
  return buf;
}

export default function () {
  const url = `ws://${TARGET_HOST}:${TARGET_PORT}`;

  const res = ws.connect(url, {}, function (socket) {
    let currentKey = null;

    function pickNewDirection() {
      if (currentKey) {
        socket.sendBinary(buildKeyInputPacket(currentKey, false));
      }
      currentKey = KEYS[Math.floor(Math.random() * KEYS.length)];
      socket.sendBinary(buildKeyInputPacket(currentKey, true));
      socket.setTimeout(pickNewDirection, MOVE_INTERVAL_MS);
    }

    socket.on('open', () => {
      // 동시 접속 스파이크를 살짝 흩어주기 위한 랜덤 시작 지연
      socket.setTimeout(pickNewDirection, Math.random() * MOVE_INTERVAL_MS);
    });

    socket.on('error', (e) => {
      console.error(`WS error: ${JSON.stringify(e)}`);
    });
  });

  check(res, { 'connected (101)': (r) => r && r.status === 101 });
}
