FROM node:20-alpine AS base
USER node
WORKDIR /home/node
COPY --chown=node:node package*.json ./

# --- Development
FROM base AS development
ENV NODE_ENV=development
RUN npm ci
COPY --chown=node:node . .
CMD ["npm", "run", "start:dev"]

# --- Production
FROM base AS builder
ENV NODE_ENV=production
RUN npm ci
COPY --chown=node:node . .
RUN npm run build && npm prune --omit=dev

FROM node:20-alpine AS production
USER node
WORKDIR /home/node
COPY --from=builder --chown=node:node /home/node/node_modules ./node_modules
COPY --from=builder --chown=node:node /home/node/dist ./dist
CMD ["node", "dist/main.js"]
