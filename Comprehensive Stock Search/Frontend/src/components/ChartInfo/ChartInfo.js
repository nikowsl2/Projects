import React from 'react';
import './ChartInfo.css';
import Highcharts from 'highcharts/highstock';
import HighchartsReact from 'highcharts-react-official';
import IndicatorsCore from 'highcharts/indicators/indicators-all';
import VBP from 'highcharts/indicators/volume-by-price';

IndicatorsCore(Highcharts);
VBP(Highcharts);

const ChartInfo = ({ companyData }) => {
    const TradeData = companyData.tradeSet || [];
    const ohlc = TradeData.map(item => [
        item.Date - 8 * 3600 * 1000,
        item.Open,
        item.High,
        item.Low,
        item.Close,
    ]);

    const volume = TradeData.map(item => [
        item.Date - 8 * 3600 * 1000,
        item.Volume,
    ]);

    const chartOptions = {
        chart: {
            backgroundColor: '#f5f5f5',
            height: 640
        },
        rangeSelector: {
            selected: 2,
        },
        title: {
            text: `${companyData.companyData.Ticker} Historical`,
        },
        subtitle: {
            text: 'With SMA and Volume by Price technical indicators'
        },
        yAxis: [
            {
                labels: {
                    align: 'right',
                    x: -3,
                },
                title: {
                    text: 'OHLC',
                },
                height: '60%',
                lineWidth: 2,
                resize: {
                    enabled: true,
                },
            },
            {
                labels: {
                    align: 'right',
                    x: -3,
                },
                title: {
                    text: 'Volume',
                },
                top: '65%',
                height: '35%',
                offset: 0,
                lineWidth: 2,
            },
        ],
        series: [
            {
                type: 'candlestick',
                name: companyData.companyData.Ticker,
                data: ohlc,
                id: 'aapl',
            },
            {
                type: 'column',
                name: 'Volume',
                data: volume,
                id: 'volume',
                yAxis: 1,
            },
            {
                type: 'vbp',
                linkedTo: 'aapl',
                params: {
                    volumeSeriesID: 'volume'
                },
                dataLabels: {
                    enabled: false
                },
                zoneLines: {
                    enabled: false
                }
            },
            {
                type: 'sma',
                linkedTo: 'aapl',
                zIndex: 1,
                marker: {
                    enabled: false
                }
            }
        ],
    };

    return (
        <div className="SMAchart-container">
            <HighchartsReact highcharts={Highcharts} constructorType={'stockChart'} options={chartOptions} containerProps={{ id: 'myChartContainer' }} />
        </div>
    );
};

export default ChartInfo;
