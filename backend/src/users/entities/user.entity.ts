import { Entity, PrimaryColumn, Column } from 'typeorm';

@Entity()
export class User {
  @PrimaryColumn({ type: 'varchar' })
  username: string;
  
  @Column()
  password: string;
};
