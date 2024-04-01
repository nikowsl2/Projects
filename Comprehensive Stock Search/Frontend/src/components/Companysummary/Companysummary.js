import React, { useState, useEffect, useRef } from 'react';
import { Button } from 'react-bootstrap';
import './Companysummary.css';

const SummaryNav = ({ onChangeView }) => {
    const [activeView, setActiveView] = useState('summary');
    const [isLeftArrowDisabled, setIsLeftArrowDisabled] = useState(true);
    const [showArrows, setShowArrows] = useState(window.innerWidth < 1200);
    const navRef = useRef(null);

    useEffect(() => {
        const handleResize = () => {
            setShowArrows(window.innerWidth < 1200);
        };
        window.addEventListener('resize', handleResize);
        return () => window.removeEventListener('resize', handleResize);
    }, []);

    const handleClick = (view) => {
        setActiveView(view);
        onChangeView(view);
    };

    const handleHorizontalScroll = (element, speed, distance, step) => {
        let scrollAmount = 0;
        const slideTimer = setInterval(() => {
            element.scrollLeft += step;
            scrollAmount += Math.abs(step);
            if (scrollAmount >= distance) {
                clearInterval(slideTimer);
            }
            setIsLeftArrowDisabled(element.scrollLeft <= 0);
        }, speed);
    };

    return (
        <div className="nav-container">
            {showArrows && (
                <Button className="navArrow leftArrow" onClick={() => handleHorizontalScroll(navRef.current, 25, 100, -10)}>
                    {'<'}
                </Button>
            )}
            <nav className="navigation" ref={navRef}>
                <button id="summaryButton" className={`navButton ${activeView === 'summary' ? 'active' : ''}`} onClick={() => handleClick('summary')}>
                    Summary
                </button>
                <button id="newsButton" className={`navButton ${activeView === 'news' ? 'active' : ''}`} onClick={() => handleClick('news')}>
                    Top News
                </button>
                <button id="chartsButton" className={`navButton ${activeView === 'charts' ? 'active' : ''}`} onClick={() => handleClick('charts')}>
                    Charts
                </button>
                <button id="insightsButton" className={`navButton ${activeView === 'insights' ? 'active' : ''}`} onClick={() => handleClick('insights')}>
                    Insights
                </button>
            </nav>
            {showArrows && (
                <Button className="navArrow rightArrow" onClick={() => handleHorizontalScroll(navRef.current, 25, 100, 10)}>
                    {'>'}
                </Button>
            )}
        </div>
    );
};

export default SummaryNav;
