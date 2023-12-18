package scanner

type Scanner interface {
	// Returns the current token.
	// TOKEN_KIND_EOF is returned if there are no more tokens.
	CurrentToken() Token

	// Go to the next token.
	NextToken()
}

// TODO: Implement this interface.
