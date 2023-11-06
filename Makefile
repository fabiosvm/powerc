
BINARY_NAME=powerc

build:
	@go build -o bin/$(BINARY_NAME) -v

test:
	@go test -v ./...

clean:
	@go clean
	@rm -f bin/$(BINARY_NAME)

.PHONY: build test clean
