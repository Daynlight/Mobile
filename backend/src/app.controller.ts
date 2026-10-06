import { Controller, Post, Body, HttpStatus, HttpCode, BadRequestException, UnauthorizedException} from '@nestjs/common';
import { AppService } from './app.service';

@Controller()
export class AppController {
  constructor(private readonly appService: AppService) {}

  @Post('login')
  @HttpCode(HttpStatus.OK)
  async login(@Body('username') username: string, @Body('password') password: string): Promise<boolean> {
    if(username === "") throw new UnauthorizedException('Invalid username');
    if(password === "") throw new UnauthorizedException('Invalid password');
    
    const user = await this.appService.login(username, password);
    if (!user) throw new UnauthorizedException('Invalid username or password');
    return user;
  };

  @Post('register')
  @HttpCode(HttpStatus.OK)
  async register(@Body('username') username: string, @Body('password') password: string, @Body('validate_password') validate_password: string) {
    if(username === "") throw new UnauthorizedException('Invalid username');
    if(password === "") throw new UnauthorizedException('Invalid password');
    if(validate_password === "") throw new UnauthorizedException('Invalid validate_password');
    
    if(validate_password !== password) throw new BadRequestException("Passwords are not the same");
    await this.appService.register(username, password, validate_password);
  };
};
