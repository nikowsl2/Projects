import React, { useState, useEffect, useRef } from 'react';
import './Autocomplete.css';
import Container from 'react-bootstrap/Container';

function debounce(func, wait) {                                                          //from Chatgpt
    let timeout;                                                                         //from Chatgpt
    return function executedFunction(...args) {                                          //from Chatgpt
        const later = () => {                                                            //from Chatgpt
            clearTimeout(timeout);                                                       //from Chatgpt
            func(...args);                                                               //from Chatgpt
        };
        clearTimeout(timeout);                                                           //from Chatgpt
        timeout = setTimeout(later, wait);                                               //from Chatgpt
    };
}

const MainPageAutocomplete = ({ onFetchData }) => {
    const [isLoading, setIsLoading] = useState(false);
    const [searchLoading, setSearchLoading] = useState(false);
    const [error, setError] = useState('');
    const [filteredSuggestions, setFilteredSuggestions] = useState([]);
    const [showSuggestions, setShowSuggestions] = useState(false);
    const [input, setInput] = useState("");
    const requestSequenceRef = useRef(0);

    const fetchSuggestions = async (userInput) => {
        if (!userInput.trim()) return;
        setIsLoading(true);

        const currentSequence = ++requestSequenceRef.current;                                       //from Chatgpt

        try {
            const response = await fetch(`https://wsl571-assignment3node-418610.wl.r.appspot.com/search-company?q=${userInput}`);
            if (!response.ok) {
                throw new Error(`Error fetching suggestions: ${response.statusText}`);
            }
            const data = await response.json();

            if (currentSequence !== requestSequenceRef.current) return;                             //from Chatgpt

            const suggestions = data.result
                ? data.result.filter(item => item.type === "Common Stock"
                    && !item.symbol.includes(".")
                    && item.symbol.startsWith(userInput))
                    .map(item => ({
                        symbol: item.displaySymbol,
                        description: item.description
                    }))
                : [];
            setFilteredSuggestions(suggestions);
        }

        catch (error) {
            console.error('Error fetching suggestions:', error);
            setFilteredSuggestions([]);
        }

        finally {
            if (currentSequence === requestSequenceRef.current) {                                   //from Chatgpt
                setIsLoading(false);
            }
        }
    };

    const debouncedFetchSuggestions = debounce(fetchSuggestions, 300);

    useEffect(() => {
        debouncedFetchSuggestions(input);
    }, [input]);

    const onChange = e => {
        const userInput = e.currentTarget.value.toUpperCase();
        setInput(userInput);
        setShowSuggestions(true);
    };

    const onKeyDown = e => {
        if (e.keyCode === 13) {
            handleSubmit(input);
            setShowSuggestions(false);
        }
    };

    const onClick = (symbol) => {
        setInput(symbol);
        handleSubmit(symbol);
        setShowSuggestions(false);
    };

    const clearInput = () => {
        setInput("");
        setFilteredSuggestions([]);
        setShowSuggestions(false);
    };

    const handleSubmit = (symbol = input) => {
        const companySymbol = String(symbol).trim();
        setSearchLoading(true);
        setError('');
        setShowSuggestions(false);

        if (!companySymbol) {
            setError('Please enter a valid ticker.');
            setSearchLoading(false);
            return;
        }

        else {
            onFetchData(companySymbol);
        }
        setSearchLoading(false);
    };

    return (
        <div className='SearchSpace'>
            <Container className="autocomplete-container">
                <div className="autocomplete">
                    <div className="input-group">
                        <input className="inputBox" type="text" onChange={onChange} value={input} onKeyDown={onKeyDown}
                            placeholder='Enter stock ticker symbol' />
                        <button className="submitButton" onClick={(e) => handleSubmit(input)}></button>
                        <button className="clearButton" onClick={clearInput}></button>
                        {showSuggestions && input && (
                            <ul className="suggestions">
                                {isLoading ? (<li className="loader"></li>)
                                    : (filteredSuggestions.map((suggestion, index) => (
                                        <li key={index} onClick={() => onClick(suggestion.symbol, suggestion.description)}>
                                            {suggestion.symbol} | {suggestion.description}
                                        </li>
                                    ))
                                    )}
                            </ul>
                        )}
                    </div>
                </div>
                {searchLoading && <div className="loader2"></div>}
                {error && (
                    <div className="error">
                        {error}
                        <button onClick={() => setError('')} className="errorCloseButton">X</button>
                    </div>
                )}
            </Container>
        </div>
    );
}

export default MainPageAutocomplete;
