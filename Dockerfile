# Minimalny obraz – tylko po to, by workflow mógł coś zbudować i wypchnąć
FROM alpine:3.20

ARG VERSION=dev
LABEL org.opencontainers.image.title="guardian-beacon"
LABEL org.opencontainers.image.description="Container placeholder for Guardian Beacon CI"
LABEL org.opencontainers.image.version="${VERSION}"

# Utwórz nieuprzywilejowanego użytkownika
RUN adduser -D -h /app app
USER app
WORKDIR /app

# (Opcjonalnie) pokaż metadane builda w logu uruchomieniowym
CMD echo "Guardian Beacon container OK (version=${VERSION})" && sleep infinity
