import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import * as fs from 'fs';
import * as path from 'path';

async function bootstrap() {
  const keyPath = process.env.SSL_KEY_PATH || path.join('/cert/key.pem');
  const certPath = process.env.SSL_CERT_PATH || path.join('/cert/cert.pem');

  const httpsOptions = { key: fs.readFileSync(keyPath), cert: fs.readFileSync(certPath) };
  const app = await NestFactory.create(AppModule, { httpsOptions });
  
  await app.listen(process.env.PORT ?? 3000);
};
bootstrap();
