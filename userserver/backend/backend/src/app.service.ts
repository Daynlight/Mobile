import { Injectable, BadRequestException, UnauthorizedException } from '@nestjs/common';
import { UsersService } from './users/users.service';

@Injectable()
export class AppService {
  constructor(private readonly usersService: UsersService) {}
  
  async login(username: string, password: string): Promise<boolean> {
    if(username === "") throw new UnauthorizedException('Invalid username');
    if(password === "") throw new UnauthorizedException('Invalid password');
    
    return await this.usersService.login({ username, password });
  };

  async register(username: string, password: string, validate_password: string) {
    if(username === "") throw new UnauthorizedException('Invalid username');
    if(password === "") throw new UnauthorizedException('Invalid password');
    if(validate_password === "") throw new UnauthorizedException('Invalid validate_password');
    if(password !== validate_password) throw new BadRequestException('Passwords are not the same');
    
    return await this.usersService.register({ username, password });
  };
};
