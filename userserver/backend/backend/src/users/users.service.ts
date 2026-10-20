import { Injectable, ConflictException } from '@nestjs/common';
import { InjectRepository } from '@nestjs/typeorm';
import { Repository } from 'typeorm';
import * as bcrypt from 'bcrypt';

import { User } from './entities/user.entity';
import { AuthDto } from './dto/auth.dto';

@Injectable()
export class UsersService {
  constructor(@InjectRepository(User) private usersRepository: Repository<User>) {}

  async register(authDto: AuthDto): Promise<User> {
    const existing = await this.usersRepository.findOne({ where: { username: authDto.username } });
    if (existing) throw new ConflictException('Username already exists');

    const saltRounds = 10;
    const hashedPassword = await bcrypt.hash(authDto.password, saltRounds);
    const user = this.usersRepository.create({ ...authDto, password: hashedPassword });
    
    return this.usersRepository.save(user);
  };

  async login(authDto: AuthDto): Promise<boolean> {
    const user = await this.usersRepository.findOne({ where: { username: authDto.username } });
    if (!user) return false;
    return await bcrypt.compare(authDto.password, user.password);
  };
};
