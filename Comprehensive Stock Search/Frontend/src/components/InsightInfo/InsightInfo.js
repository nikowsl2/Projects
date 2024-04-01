import React from 'react';
import './InsightInfo.css';
import Highcharts from 'highcharts';
import HighchartsReact from 'highcharts-react-official';
import { Container, Row, Col } from 'react-bootstrap';

const InsightInfo = ({ companyData }) => {
    console.log('companyData:', companyData);
    const InsightData = companyData.companyInsider || [];
    const { CompanysName } = companyData.companyData;
    const TrendData = companyData.recommendationTrend;
    const EarningData = companyData.companyEarnings || [];
    const series = [
        {
            name: 'Strong Buy',
            data: TrendData.map(item => ({
                y: item.strongBuy,
                period: item.period,
            })),
            color: '#206b00'
        },
        {
            name: 'Buy',
            data: TrendData.map(item => ({
                y: item.Buy,
                period: item.period
            })),
            color: '#3fc904'
        },
        {
            name: 'Hold',
            data: TrendData.map(item => ({
                y: item.Hold,
                period: item.period
            })),
            color: '#a87402'
        },
        {
            name: 'Sell',
            data: TrendData.map(item => ({
                y: item.sell,
                period: item.period
            })),
            color: '#eb0000'
        },
        {
            name: 'Strong Sell',
            data: TrendData.map(item => ({
                y: item.strongSell,
                period: item.period
            })),
            color: '#4d0202'
        }
    ];

    const EarningSeriesData = companyData.companyEarnings.map(item => ({
        period: item.period,
        actual: item.actual,
        estimate: item.estimate,
        surprise: item.surprise
    }));

    const categories = TrendData.map(item => item.period.slice(0, 7));

    const sumChange = InsightData.reduce((accumulator, currentItem) => {
        return accumulator + currentItem.change;
    }, 0);

    const sumPositiveChange = InsightData.reduce((accumulator, currentItem) => {
        if (currentItem.change > 0) {
            return accumulator + currentItem.change;
        };
        return accumulator;
    }, 0);

    const sumNegativeChange = InsightData.reduce((accumulator, currentItem) => {
        if (currentItem.change < 0) {
            return accumulator + currentItem.change;
        };
        return accumulator;
    }, 0);

    const sumMspr = InsightData.reduce((accumulator, currentItem) => {
        return accumulator + currentItem.mspr;
    }, 0);

    const sumPositiveMspr = InsightData.reduce((accumulator, currentItem) => {
        if (currentItem.mspr > 0) {
            return accumulator + currentItem.mspr;
        };
        return accumulator;
    }, 0);

    const sumNegativeMspr = InsightData.reduce((accumulator, currentItem) => {
        if (currentItem.mspr < 0) {
            return accumulator + currentItem.mspr;
        };
        return accumulator;
    }, 0);

    const formatPrice = (price) => isNaN(price) ? 'N/A' : Number(price).toFixed(2);

    const chartOptions = {
        chart: {
            type: 'column',
            marginBottom: 100,
            backgroundColor: '#f5f5f5',
        },
        title: {
            text: 'Recommendation Trends',
            align: 'center'
        },
        xAxis: {
            categories
        },
        yAxis: {
            min: 0,
            title: {
                text: '# Analysis'
            },
            stackLabels: {
                enabled: false
            }
        },
        legend: {
            align: 'center',
            verticalAlign: 'bottom',
            x: 0,
            y: 0,
            backgroundColor: Highcharts.defaultOptions.legend.backgroundColor || '#f5f5f5',
            borderColor: '#f5f5f5',
            borderWidth: 0,
            shadow: false
        },
        tooltip: {
            headerFormat: '<b>{point.x}</b><br/>',
            pointFormat: '{series.name}: {point.y}<br/>Total: {point.stackTotal}'
        },
        plotOptions: {
            column: {
                stacking: 'normal',
                dataLabels: {
                    enabled: true
                }
            }
        },
        series: series
    }

    const categorie2 = EarningData.map(item => `${item.period}<br> Surprise: ${item.surprise}`);
    const chartOptions2 = {
        chart: {
            type: 'spline',
            backgroundColor: '#f5f5f5',
            events: {                                                               //from ChatGPT
                load: function () {                                                 //from ChatGPT
                    var chart = this,                                               //from ChatGPT
                        plotWidth = chart.plotWidth,
                        plotLeft = chart.plotLeft,
                        yAxis = chart.yAxis[0],                                     //from ChatGPT
                        x1 = plotLeft,                                              //from ChatGPT
                        x2 = plotLeft + plotWidth,
                        y = yAxis.toPixels(yAxis.max) + 300;                        //from ChatGPT
                    chart.renderer.path(['M', x1, y, 'L', x2, y])                   //from ChatGPT
                        .attr({                                                     //from ChatGPT
                            'stroke-width': 1,                                      //from ChatGPT
                            stroke: 'black',                                        //from ChatGPT
                            zIndex: 5                                               //from ChatGPT
                        })                                                          //from ChatGPT
                        .add();                                                     //from ChatGPT
                }
            }
        },
        title: {
            text: 'Historical ESP Surprises',
            align: 'center'
        },
        xAxis: {
            categories: categorie2,
            reversed: false,
            maxPadding: 0.05,
            showLastLabel: true,
        },
        yAxis: {
            title: {
                text: 'Quarterly EPS'
            },
            labels: {
                format: '{value}'
            },
            lineWidth: 0
        },
        legend: {
            enabled: true
        },
        plotOptions: {
            spline: {
                marker: {
                    enable: false
                }
            }
        },
        series: [
            {
                name: 'Actual',
                data: EarningSeriesData.map(item => item.actual),
            },
            {
                name: 'Estimate',
                data: EarningSeriesData.map(item => item.estimate),
            },
        ],
    }

    return (
        <div className="Insight-container">
            <p className="Insight-tabletitle">Insider Sentiments</p>
            <table className="Insight-table">
                <thead>
                    <tr>
                        <th>{CompanysName}</th>
                        <th scope="col">MSPR</th>
                        <th scope="col">Change</th>
                    </tr>
                </thead>
                <tbody>
                    <tr className="connected-line">
                        <td colSpan="3"></td>
                    </tr>
                    <tr>
                        <th scope="row">Total</th>
                        <td>{formatPrice(sumMspr)}</td>
                        <td>{formatPrice(sumChange)}</td>
                    </tr>
                    <tr className="connected-line">
                        <td colSpan="3"></td>
                    </tr>
                    <tr>
                        <th scope="row">Positive</th>
                        <td>{formatPrice(sumPositiveMspr)}</td>
                        <td>{formatPrice(sumPositiveChange)}</td>
                    </tr>
                    <tr className="connected-line">
                        <td colSpan="3"></td>
                    </tr>
                    <tr>
                        <th scope="row">Negative</th>
                        <td>{formatPrice(sumNegativeMspr)}</td>
                        <td>{formatPrice(sumNegativeChange)}</td>
                    </tr>
                    <tr className="connected-line">
                        <td colSpan="3"></td>
                    </tr>
                </tbody>
            </table>
            <Container>
                <Row className="Insightchart-container">
                    <Col xs={12} lg={6}>
                        <div className="chart-wrapper">
                            <HighchartsReact highcharts={Highcharts} options={chartOptions} />
                        </div>
                    </Col>
                    <Col xs={12} lg={6}>
                        <div className="chart-wrapper">
                            <HighchartsReact highcharts={Highcharts} options={chartOptions2} />
                        </div>
                    </Col>
                </Row>

            </Container>
        </div>
    );
};

export default InsightInfo;