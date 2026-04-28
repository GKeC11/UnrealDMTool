import { GameServer } from "./GameServer";
import { Logger } from "./Utils/Logger";

const logger = new Logger("Server");
const port = Number(process.env.WS_PORT || process.env.PORT || 7788);

const gameServer = new GameServer({
    port,
    path: "/ws",
});

gameServer.start();
logger.info(`GameServer bootstrapped. port=${port}`);
